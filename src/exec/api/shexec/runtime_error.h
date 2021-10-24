#pragma once

#include "utils/error.h"

/**
** \details runtime_error is thrown when an error preventing the script from continuing occurs,
** such as fork failing.
*/
extern struct ex_class g_runtime_error;

__noreturn static inline void runtime_error(struct ex_scope *scope, int code)
{
    scope->context->retcode = code;
    shraise(scope, &g_runtime_error);
}
