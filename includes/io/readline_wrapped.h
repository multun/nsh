#pragma once

#include "utils/error.h"

void readline_wrapped_setup(void);
char *readline_wrapped(struct ex_scope *ex_scope, const char *prompt);
