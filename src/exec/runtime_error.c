#include <nsh_exec/runtime_error.h>


struct exception_type g_runtime_error = {
    .compat_error = NSH_EXECUTION_ERROR,
};
