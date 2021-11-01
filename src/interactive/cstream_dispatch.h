#pragma once

#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>


int cstream_dispatch_init(struct repl *repl, struct cstream **cs,
                          struct cli_options *arg_cont);
