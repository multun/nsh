#include "shexec/clean_exit.h"

#include <err.h>
#include <errno.h>
#include <stdarg.h>

struct ex_class g_clean_exit;

void ATTR(noreturn) clean_err(struct ex_scope *ex_scope, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);
    clean_exit(ex_scope, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_errx(struct ex_scope *ex_scope, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);
    clean_exit(ex_scope, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_exit(struct ex_scope *ex_scope, int retcode)
{
    ex_scope->context->retcode = retcode;
    shraise(ex_scope, &g_clean_exit);
}
