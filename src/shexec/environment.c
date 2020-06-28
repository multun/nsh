#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <err.h>

#include "shparse/ast.h"
#include "repl/repl.h"
#include "shexec/args.h"
#include "shexec/builtin_cd.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"
#include "utils/macros.h"
#include "utils/mprintf.h"
#include "utils/safe_syscalls.h"


static char **arg_context_extract(int *target_argc, struct arg_context *args)
{
    int arguments_count = args->argc - args->argc_base;
    // don't forget $0 in the total argument count
    int argc = arguments_count + 1;
    *target_argc = argc;

    // the other one is the terminating NULL
    char **ret = xcalloc(sizeof(char **), argc + 1);
    ret[0] = strdup(args->argv[args->progname_ind]);
    for (int i = 0; i < arguments_count; i++)
        ret[i + 1] = strdup(args->argv[args->argc_base + i]);
    // no need to add the terminating null, as we already calloced
    return ret;
}

char **environment_array(struct environment *env)
{
    char **res = xmalloc((env->variables.size + 1) * sizeof(*res));
    res[env->variables.size] = NULL;
    size_t i = 0;
    struct hash_table_it it;
    for_each_hash(it, &env->variables)
    {
        struct shexec_variable *var = container_of(it.cur, struct shexec_variable, hash);
        if (var->value == NULL)
            continue;

        struct sh_value *value = var->value;
        if (!sh_value_is_string(value))
            continue;

        struct sh_string *str_value = (struct sh_string*)value;
        res[i] = mprintf("%s=%s", hash_head_key(it.cur), sh_string_data(str_value));
        i++;
    }
    assert(i == env->variables.size);
    return res;
}

static void environment_load_variables(struct environment *env)
{
    for (size_t i = 0; environ[i]; i++) {
        char *var = environ[i];
        char *eq = strchr(var, '=');
        char *name = strndup(var, eq - var);
        char *value = strdup(eq + 1);
        environment_var_assign_cstring(env, name, value, true);
    }

    const char *PWD = environment_var_get_cstring(env, "PWD");
    if (PWD == NULL) {
        char *pwd;
        if ((pwd = safe_getcwd()) == NULL)
            warn("getcwd() failed");
        else
            environment_var_assign_cstring(env, strdup("PWD"), pwd, true);
    }

    if (environment_var_get(env, "IFS") == NULL)
        environment_var_assign_cstring(env, strdup("IFS"), strdup("\t\n "), true);
}

static void var_free(struct hash_head *head)
{
    struct shexec_variable *var = container_of(head, struct shexec_variable, hash);
    free(hash_head_key(head));
    sh_value_put(var->value);
    free(var);
}

struct sh_value *environment_var_get(struct environment *env, const char *name)
{
    struct hash_head *prev = hash_table_find(&env->variables, NULL, name);
    if (prev == NULL)
        return NULL;

    struct shexec_variable *var = container_of(prev, struct shexec_variable, hash);
    return var->value;
}

void environment_var_assign(struct environment *env, char *name, struct sh_value *value, bool export)
{
    struct hash_head **insertion_pos;
    struct hash_head *prev = hash_table_find(&env->variables, &insertion_pos, name);
    if (prev) {
        struct shexec_variable *var = container_of(prev, struct shexec_variable, hash);
        sh_value_put(var->value);
        free(name);
        var->value = value;
        if (export)
            var->exported = true;
        return;
    }

    struct shexec_variable *nvar = xmalloc(sizeof(*nvar));
    hash_head_init(&nvar->hash, name);
    nvar->value = value;
    nvar->exported = export;
    hash_table_insert(&env->variables, insertion_pos, &nvar->hash);
}

static void function_hash_put(struct hash_head *head)
{
    struct shast_function *func = container_of(head, struct shast_function, hash);
    shast_ref_put(&func->base);
}

static void environment_free(struct refcnt *refcnt)
{
    struct environment *env = (struct environment *)refcnt;
    free(env->progname);
    argv_free(env->argc, env->argv);
    hash_table_map(&env->variables, var_free);
    hash_table_map(&env->functions, function_hash_put);
    hash_table_destroy(&env->variables);
    hash_table_destroy(&env->functions);
    free(env);
}

struct environment *environment_create(struct arg_context *arg_cont)
{
    struct environment *env = zalloc(sizeof(*env));
    ref_init(&env->refcnt, environment_free);
    signal_manager_init(&env->sigman);
    env->argv = arg_context_extract(&env->argc, arg_cont);
    env->progname = strdup(arg_cont->argv[arg_cont->progname_ind]);
    hash_table_init(&env->variables, 10);
    hash_table_init(&env->functions, 10);
    env->code = 0;
    env->forked = false;

    env->break_count = 0;
    env->depth = 0;
    environment_load_variables(env);
    return env;
}
