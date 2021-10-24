#include <nsh_exec/environment.h>
#include <nsh_exec/clean_exit.h>

#include <err.h>
#include <errno.h>
#include <stdarg.h>

struct exception_type g_clean_exit;

void ATTR(noreturn) clean_err(struct exception_catcher *catcher, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);
    clean_exit(catcher, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_errx(struct exception_catcher *catcher, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);
    clean_exit(catcher, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_exit(struct exception_catcher *catcher, int retcode)
{
    catcher->context->retcode = retcode;
    shraise(catcher, &g_clean_exit);
}


int builtin_exit(struct environment *env, struct exception_catcher *catcher, int argc, char **argv)
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

    clean_exit(catcher, rc);
}
