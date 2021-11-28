#include <nsh_exec/environment.h>
#include <nsh_exec/clean_exit.h>

#include <err.h>
#include <errno.h>
#include <stdarg.h>

struct exception_type g_clean_exit = {
    .compat_error = NSH_EXIT_INTERUPT,
};

__unused_result int clean_err(struct environment *env, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);

    va_end(ap);

    return clean_exit(env, retcode);
}

__unused_result int clean_errx(struct environment *env, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);

    va_end(ap);

    return clean_exit(env, retcode);
}

__unused_result int clean_exit(struct environment *env, int retcode)
{
    env->code = retcode;
    return NSH_EXIT_INTERUPT;
}


int builtin_exit(struct environment *env, int argc, char **argv)
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

    return clean_exit(env, rc);
}
