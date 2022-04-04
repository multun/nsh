#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <err.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/repl.h>
#include <nsh_exec/environment.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/hashmap.h>
#include <nsh_utils/hashmap_iterators.h>
#include <nsh_utils/macros.h>
#include <nsh_utils/mprintf.h>
#include <nsh_utils/safe_syscalls.h>

#include "args.h"


static char **arg_context_extract(int *target_argc, struct cli_options *args)
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
    struct cpvect res;
    cpvect_init(&res, env->variables.size + 1);
    struct hashmap_it it;
    for_each_hash(it, &env->variables)
    {
        struct shexec_variable *var = container_of(it.cur, struct shexec_variable, hash);
        if (var->value == NULL)
            continue;

        if (!var->exported)
            continue;

        struct sh_value *value = var->value;
        if (!sh_value_is_string(value))
            continue;

        struct sh_string *str_value = (struct sh_string *)value;
        cpvect_push(&res, mprintf("%s=%s", it.cur->key, sh_string_data(str_value)));
    }
    cpvect_push(&res, NULL);
    return cpvect_data(&res);
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
        environment_var_assign_cstring(env, strdup("IFS"), strdup(" \t\n"), false);
}

static void var_free(struct hashmap_item *head)
{
    struct shexec_variable *var = container_of(head, struct shexec_variable, hash);
    free(var->hash.key);
    sh_value_put(var->value);
    free(var);
}

struct sh_value *environment_var_get(struct environment *env, const char *name)
{
    struct hashmap_item *prev = hashmap_find(&env->variables, NULL, name);
    if (prev == NULL)
        return NULL;

    struct shexec_variable *var = container_of(prev, struct shexec_variable, hash);
    return var->value;
}

void environment_var_assign(struct environment *env, char *name, struct sh_value *value,
                            bool export)
{
    struct hashmap_item **insertion_pos;
    struct hashmap_item *prev = hashmap_find(&env->variables, &insertion_pos, name);
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
    hashmap_item_init(&nvar->hash, name);
    nvar->value = value;
    nvar->exported = export;
    hashmap_insert(&env->variables, insertion_pos, &nvar->hash);
}

static void function_hash_put(struct hashmap_item *head)
{
    struct shast_function *func = container_of(head, struct shast_function, hash);
    shast_ref_put(&func->base);
}

static void environment_free(struct refcnt *refcnt)
{
    struct environment *env = (struct environment *)refcnt;
    free(env->progname);
    argv_free(env->argc, env->argv);
    hashmap_apply(&env->variables, var_free);
    hashmap_apply(&env->functions, function_hash_put);
    hashmap_destroy(&env->variables);
    hashmap_destroy(&env->functions);
    free(env);
}

struct environment *environment_load(struct cli_options *arg_cont)
{
    struct environment *env = zalloc(sizeof(*env));
    ref_init(&env->refcnt, environment_free);
    signal_manager_init(&env->sigman);
    env->argv = arg_context_extract(&env->argc, arg_cont);
    env->progname = strdup(arg_cont->argv[arg_cont->progname_ind]);
    memcpy(env->shopts, arg_cont->shopts, sizeof(arg_cont->shopts));
    hashmap_init(&env->variables, 10);
    hashmap_init(&env->functions, 10);
    env->code = 0;
    env->forked = false;
    env->find_builtin = find_default_builtin;

    env->break_count = 0;
    env->depth = 0;
    environment_load_variables(env);
    return env;
}
