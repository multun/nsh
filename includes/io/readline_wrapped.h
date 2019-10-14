#pragma once

#include "utils/error.h"

void readline_wrapped_setup(void);
char *readline_wrapped(struct errcont *errcont, const char *prompt);
