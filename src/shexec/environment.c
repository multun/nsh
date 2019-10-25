#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "repl/repl.h"
#include "shexec/args.h"
#include "shexec/builtin_cd.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"

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

struct environment*environment_create(struct arg_context *arg_cont)
{
    struct environment *env = xmalloc(sizeof(*env));
    env->argv = arg_context_extract(&env->argc, arg_cont);
    env->progname = strdup(arg_cont->argv[arg_cont->progname_ind]);
    env->vars = htable_create(10);
    env->functions = htable_create(10);
    env->code = 0;

    env->break_count = 0;
    env->depth = 0;
    environment_load(env);
    return env;
}

static bool vartoenv(struct pair *node, char **pos)
{
    struct variable *var = node->value;
    if (!var->to_export)
        return false;
    size_t size = strlen(node->key) + 1;
    if (var->value && *var->value) {
        size += strlen(var->value) + 1;
        *pos = xmalloc(size * sizeof(char));
        sprintf(*pos, "%s=%s", node->key, var->value);
    } else {
        *pos = xmalloc(size * sizeof(char));
        sprintf(*pos, "%s", node->key);
    }
    return true;
}

char **environment_array(struct environment *env)
{
    char **res = xmalloc((env->vars->size + 1) * sizeof(*res));
    size_t pos = 0;
    for (size_t i = 0; i < env->vars->capacity; i++) {
        struct pair *pp = NULL;
        struct pair *fp = env->vars->tab[i];
        while (fp || pp) {
            if (pp)
                pos += vartoenv(pp, res + pos);
            pp = fp;
            if (fp)
                fp = fp->next;
        }
    }
    res[pos] = NULL;
    return res;
}

void environment_load(struct environment *env)
{
    for (char **it = environ; *it; it++) {
        char *var = strdup(*it);
        char *save = NULL;
        char *name = strtok_r(var, "=", &save);
        char *value = strtok_r(NULL, "\0", &save);
        if (!value)
            value = xcalloc(1, sizeof(char));
        else
            value = strdup(value);
        environment_var_assign(env, name, value, true);
        struct pair *p = htable_access(env->vars, name);
        struct variable *node = p->value;
        node->to_export = true;
    }
    if (!htable_access(env->vars, "PWD"))
        update_pwd(false, env);
    if (!htable_access(env->vars, "IFS"))
        environment_var_assign(env, strdup("IFS"), strdup("\t\n "), true);
}

static void var_free(struct pair *p)
{
    free(p->key);
    struct variable *var = p->value;
    free(var->value);
    free(var);
}

void environment_free(struct environment *env)
{
    if (!env)
        return;

    free(env->progname);
    htable_map(env->vars, var_free);
    argv_free(env->argv);
    htable_free(env->vars);
    htable_free(env->functions);
    free(env);
}

void environment_var_assign(struct environment *env, char *name, char *value, bool export)
{
    struct pair *prev = htable_access(env->vars, name);
    if (prev) {
        struct variable *var = prev->value;
        free(var->value);
        free(name);
        var->value = value;
        if (export)
            var->to_export = true;
        return;
    }
    struct variable *nvar = xmalloc(sizeof(*nvar));
    *nvar = VARIABLE(value);
    nvar->to_export = export;
    htable_add(env->vars, name, nvar);
}
