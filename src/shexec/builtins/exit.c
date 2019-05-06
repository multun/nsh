#include <err.h>

#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "utils/alloc.h"

int builtin_exit(s_env *env, s_errcont *cont, int argc, char **argv)
{
    if (!env)
        warnx("exit: missing context elements");

    if (argc > 2) {
        warnx("exit: too many arguments");
        return 1;
    }

    int rc;
    if (argc == 2) {
        char *invalid_char;
        rc = strtol(argv[1], &invalid_char, 10);
        if (!*argv[1] || (invalid_char && *invalid_char))
            rc = 2;
    } else
        rc = env->code;

    clean_exit(cont, rc);
}
