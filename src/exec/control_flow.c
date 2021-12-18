#include <nsh_exec/ast_exec.h>
#include <nsh_exec/expansion.h>
#include <nsh_exec/environment.h>
#include <nsh_utils/alloc.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>

#include "execution_error.h"


int if_exec(struct environment *env, struct shast *ast)
{
    int rc;
    struct shast_if *if_node = (struct shast_if *)ast;

    if ((rc = ast_exec(env, if_node->condition)))
        return rc;

    if (!env->code)
        return ast_exec(env, if_node->branch_true);
    else if (if_node->branch_false)
        return ast_exec(env, if_node->branch_false);
    return NSH_OK;
}


nsh_err_t case_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_case *case_node = (struct shast_case *)ast;
    char *case_var;

    if ((err = expand_nosplit(&case_var, &case_node->base.line_info,
                              shword_buf(case_node->var), env, 0)))
        return err;

    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++) {
            char *pattern = shword_buf(wordlist_get(&case_item->pattern, i));
            if (fnmatch(pattern, case_var, 0) != 0)
                continue;

            free(case_var);
            return ast_exec(env, case_item->action);
        }
    }

    free(case_var);
    return err;
}


static nsh_err_t builtin_generic_break(nsh_err_t break_err, struct environment *env,
                                       int argc, char **argv)
{
    if (argc > 2) {
        warnx("%s: too many arguments", argv[0]);
        return execution_error(env, 1);
    }

    if (env->depth == 0) {
        warnx("%s: only meaningful in a loop", argv[0]);
        return execution_error(env, 1);
    }

    if (argc < 2)
        env->break_count = 1;
    else {
        unsigned long int res = strtoul(argv[1], NULL, 10);
        if (res == ULONG_MAX || res == 0) {
            warnx("%s: `%s': invalid break count", argv[0], argv[1]);
            return execution_error(env, 1);
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
    return break_err;
}


int builtin_break(struct environment *env, int argc, char **argv)
{
    return builtin_generic_break(NSH_BREAK_INTERUPT, env, argc, argv);
}


int builtin_continue(struct environment *env, int argc, char **argv)
{
    return builtin_generic_break(NSH_CONTINUE_INTERUPT, env, argc, argv);
}
