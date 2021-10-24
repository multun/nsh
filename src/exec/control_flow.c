#include <nsh_exec/ast_exec.h>
#include <nsh_exec/expansion.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_utils/alloc.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>

#include "break.h"


int if_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    int cond = ast_exec(env, if_node->condition, ex_scope);
    if (!cond)
        return ast_exec(env, if_node->branch_true, ex_scope);
    else if (if_node->branch_false)
        return ast_exec(env, if_node->branch_false, ex_scope);
    return 0;
}


int case_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    char *case_var = expand_nosplit(&case_node->base.line_info, shword_buf(case_node->var), 0, env, ex_scope);
    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            char *pattern = shword_buf(wordlist_get(&case_item->pattern, i));
            if (fnmatch(pattern, case_var, 0) != 0)
                continue;

            free(case_var);
            return ast_exec(env, case_item->action, ex_scope);
        }
    }
    free(case_var);
    return 0;
}


/* break and continue are implemented as builtins, which raise exceptions */
struct ex_class g_ex_break;
struct ex_class g_ex_continue;


static int builtin_generic_break(struct environment *env,
                                 struct ex_scope *ex_scope,
                                 int argc, char **argv)
{
    if (argc > 2) {
        warnx("%s: too many arguments", argv[0]);
        runtime_error(ex_scope, 1);
    }

    if (env->depth == 0) {
        warnx("%s: only meaningful in a loop", argv[0]);
        runtime_error(ex_scope, 1);
    }

    if (argc < 2)
        env->break_count = 1;
    else {
        unsigned long int res = strtoul(argv[1], NULL, 10);
        if (res == ULONG_MAX || res == 0) {
            warnx("%s: `%s': invalid break count", argv[0], argv[1]);
            runtime_error(ex_scope, 1);
        }

        /* clamp to INT_MAX to make the convertion safe */
        if (res > INT_MAX)
            res = INT_MAX;

        env->break_count = res;
    }

    /* TODO: this is required by posix but deserves a warning */
    if (env->break_count > env->depth)
        env->break_count = env->depth;

    env->code = 0;
    /* the special -1 return code tells the wrapper to raise an exception */
    return -1;
}


int builtin_break(struct environment *env, struct ex_scope *ex_scope,
                  int argc, char **argv)
{
    int rc;
    if ((rc = builtin_generic_break(env, ex_scope, argc, argv)) >= 0)
        return rc;
    shraise(ex_scope, &g_ex_break);
}


int builtin_continue(struct environment *env, struct ex_scope *ex_scope,
                     int argc, char **argv)
{
    int rc;
    if ((rc = builtin_generic_break(env, ex_scope, argc, argv)) >= 0)
        return rc;
    shraise(ex_scope, &g_ex_continue);
}
