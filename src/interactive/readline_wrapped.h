#pragma once

#include <nsh_exec/repl.h>


void readline_wrapped_setup(void);
nsh_err_t readline_wrapped(char **res, struct environment *env, char *prompt);
