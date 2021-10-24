#pragma once

#include <nsh_utils/exception.h>

/**
** \details runtime_error is thrown when an error preventing the script from continuing occurs,
** such as fork failing.
*/
extern struct exception_type g_runtime_error;

__noreturn static inline void runtime_error(struct exception_catcher *scope, int code)
{
    scope->context->retcode = code;
    shraise(scope, &g_runtime_error);
}
