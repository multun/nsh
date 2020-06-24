#include <err.h>
#include <limits.h>

#include "shexec/break.h"
#include "shexec/builtins.h"
#include "utils/alloc.h"

static int builtin_generic_break(struct environment *env, int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return 1;
    }

    if (!env->depth)
        return 0;

    if (argc < 2)
        env->break_count = 1;
    else {
        unsigned long int res = strtoul(argv[1], NULL, 10);
        if (res == ULONG_MAX || res == 0) {
            fprintf(stderr, "%s: invalid break count \"%s\"\n", argv[0], argv[1]);
            return 1;
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
    if ((rc = builtin_generic_break(env, argc, argv)) >= 0)
        return rc;
    shraise(ex_scope, &g_ex_break);
}

int builtin_continue(struct environment *env, struct ex_scope *ex_scope,
                     int argc, char **argv)
{
    int rc;
    if ((rc = builtin_generic_break(env, argc, argv)) >= 0)
        return rc;
    shraise(ex_scope, &g_ex_continue);
}
