#include <err.h>
#include <limits.h>

#include "shexec/break.h"
#include "shexec/builtins.h"
#include "utils/alloc.h"

static int builtin_generic_break(struct environment *env, int argc, char **argv)
{
    if (argc > 2) {
        warnx("%s: too many arguments", argv[0]);
        return 1;
    }

    if (!env->depth)
        return 0;

    if (argc < 2)
        env->break_count = 1;
    else {
        unsigned long int res = strtoul(argv[1], NULL, 10);
        if (res == ULONG_MAX || res == 0 || res > INT_MAX) {
            warnx("%s: invalid break count \"%s\"", argv[0], argv[1]);
            return 1;
        }
        env->break_count = res;
    }

    /* TODO: this is posix-correct but deserves a warning */
    if (env->break_count > env->depth)
        env->break_count = env->depth;

    env->code = 0;
    return 0;
}

int builtin_break(struct environment *env, struct errcont *cont,
                  int argc, char **argv)
{
    int rc;
    if ((rc = builtin_generic_break(env, argc, argv)))
        return rc;
    shraise(cont, &g_ex_break);
}

int builtin_continue(struct environment *env, struct errcont *cont,
                     int argc, char **argv)
{
    int rc;
    if ((rc = builtin_generic_break(env, argc, argv)))
        return rc;
    shraise(cont, &g_ex_continue);
}
