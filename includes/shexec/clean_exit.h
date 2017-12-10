#pragma once

#include "utils/attr.h"
#include "utils/error.h"

extern s_ex_class g_clean_exit;

void ATTR(noreturn) clean_err(s_errcont *cont, int retcode,
                              const char *fmt, ...);
void ATTR(noreturn) clean_exit(s_errcont *cont, int retcode);
