#include <err.h>
#include <limits.h>

#include "shexec/break.h"
#include "shexec/builtins.h"
#include "utils/alloc.h"

#define BREAK_BASE(Name, Continue)                                                       \
    int builtin_##Name(struct environment *env, struct errcont *cont, int argc, char **argv)               \
    {                                                                                    \
        if (argc > 2) {                                                                  \
            warnx(#Name ": too many arguments");                                         \
            return 1;                                                                    \
        }                                                                                \
                                                                                         \
        if (!env->depth)                                                                 \
            return 0;                                                                    \
                                                                                         \
        if (argc < 2)                                                                    \
            env->break_count = 1;                                                        \
                                                                                         \
        else if ((env->break_count = strtoul(argv[1], NULL, 10)) == ULONG_MAX) {         \
            warnx(#Name ": invalid break count \"%s\"", argv[1]);                        \
            return 1;                                                                    \
        }                                                                                \
                                                                                         \
        env->code = 0;                                                                   \
        env->break_continue = Continue;                                                  \
        shraise(cont, &g_lbreak);                                                        \
    }

BREAK_BASE(break, false)
BREAK_BASE(continue, true)
