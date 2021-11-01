#pragma once

#include <nsh_exec/repl.h>


int interactive_repl_init(struct repl *repl, struct cli_options *options,
                          struct cstream **cs);
