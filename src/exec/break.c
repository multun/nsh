#include "shexec/clean_exit.h"

#include <err.h>
#include <errno.h>
#include <stdarg.h>

struct ex_class g_ex_break;
struct ex_class g_ex_continue;
