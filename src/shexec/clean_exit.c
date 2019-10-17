#include "shexec/clean_exit.h"

#include <err.h>
#include <errno.h>
#include <stdarg.h>

struct ex_class g_clean_exit;

void ATTR(noreturn) clean_err(struct errcont *cont, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);
    clean_exit(cont, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_errx(struct errcont *cont, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);
    clean_exit(cont, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_exit(struct errcont *cont, int retcode)
{
    cont->errman->retcode = retcode;
    shraise(cont, &g_clean_exit);
}
