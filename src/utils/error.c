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

const char *nsh_error_repr(enum nsh_error err)
{
    switch (err) {
    case NSH_OK:
        return "no error";
    case NSH_IO_ERROR:
        return "IO error";
    case NSH_LEXER_ERROR:
        return "lexer error";
    case NSH_PARSER_ERROR:
        return "parser error";
    case NSH_EXECUTION_ERROR:
        return "execution error";
    case NSH_EXPANSION_ERROR:
        return "expansion error";
    case NSH_KEYBOARD_INTERUPT:
        return "keyboard interupt";
    case NSH_BREAK_INTERUPT:
        return "break interupt";
    case NSH_CONTINUE_INTERUPT:
        return "continue interupt";
    case NSH_EXIT_INTERUPT:
        return "exit interupt";
    }
    return "unknown error";
}
