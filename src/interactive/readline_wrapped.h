#pragma once

#include <nsh_utils/exception.h>

void readline_wrapped_setup(void);
char *readline_wrapped(struct exception_catcher *catcher, char *prompt);
