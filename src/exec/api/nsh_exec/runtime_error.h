#pragma once

#include <nsh_exec/repl.h>
#include <nsh_utils/exception.h>

/**
** \details execution_error is thrown when an error preventing the script from continuing occurs,
** such as fork failing.
*/
extern struct exception_type g_runtime_error;

__unused_result static inline nsh_err_t execution_error(struct environment *env, int code)
{
    env->code = code;
    return NSH_EXECUTION_ERROR;
}
