#pragma once

#include <nsh_utils/exception.h>
#include <nsh_exec/repl.h>


void readline_wrapped_setup(void);
char *readline_wrapped(struct environment *env, struct exception_catcher *catcher,
                       char *prompt);
