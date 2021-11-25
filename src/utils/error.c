#include <nsh_utils/error.h>

#include <err.h>
#include <stdarg.h>

nsh_err_t error_warn(nsh_err_t err, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarn(fmt, ap);

    va_end(ap);
    return err;
}

nsh_err_t error_warnx(nsh_err_t err, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vwarnx(fmt, ap);

    va_end(ap);
    return err;
}
