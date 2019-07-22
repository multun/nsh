#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "ast/assignment.h"
#include "ast/ast_list.h"
#include "repl/repl.h"
#include "shexec/args.h"
#include "shexec/builtin_cd.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/variable.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"

static char **arg_context_extract(int *target_argc, s_arg_context *args)
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

s_env *environment_create(s_arg_context *arg_cont)
{
    s_env *env = xmalloc(sizeof(s_env));
    env->argv = arg_context_extract(&env->argc, arg_cont);
    env->progname = strdup(arg_cont->argv[arg_cont->progname_ind]);
    env->vars = htable_create(10);
    env->functions = htable_create(10);
    env->ast_list = NULL;
    env->code = 0;

    env->break_count = 0;
    env->depth = 0;
    environment_load(env);
    return env;
}

static bool vartoenv(struct pair *node, char **pos)
{
    s_var *var = node->value;
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

char **environment_array(s_env *env)
{
    char **res = xmalloc((env->vars->size + 1) * sizeof(char *));
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

void environment_load(s_env *env)
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
        assign_var(env, name, value, true);
        struct pair *p = htable_access(env->vars, name);
        s_var *node = p->value;
        node->to_export = true;
    }
    if (!htable_access(env->vars, "PWD"))
        update_pwd(false, env);
    if (!htable_access(env->vars, "IFS"))
        assign_var(env, strdup("IFS"), strdup("\t\n "), true);
}

static void var_free(struct pair *p)
{
    free(p->key);
    s_var *var = p->value;
    free(var->value);
    free(var);
}

void environment_free(s_env *env)
{
    if (!env)
        return;

    free(env->progname);
    ast_list_free(env->ast_list);
    htable_map(env->vars, var_free);
    argv_free(env->argv);
    htable_free(env->vars);
    htable_free(env->functions);
    free(env);
}
