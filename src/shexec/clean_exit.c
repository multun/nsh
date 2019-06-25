#include "shexec/clean_exit.h"

#include <err.h>
#include <errno.h>
#include <stdarg.h>

s_ex_class g_clean_exit;

void ATTR(noreturn) clean_err(s_errcont *cont, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);
    clean_exit(cont, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_errx(s_errcont *cont, int retcode, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);
    clean_exit(cont, retcode);

    va_end(ap);
}

void ATTR(noreturn) clean_exit(s_errcont *cont, int retcode)
{
    cont->errman->retcode = retcode;
    shraise(cont, &g_clean_exit);
}
