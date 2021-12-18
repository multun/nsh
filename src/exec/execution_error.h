#pragma once

#include <nsh_exec/repl.h>


__unused_result static inline nsh_err_t execution_error(struct environment *env, int code)
{
    env->code = code;
    return NSH_EXECUTION_ERROR;
}
