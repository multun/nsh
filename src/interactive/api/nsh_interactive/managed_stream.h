#pragma once

#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>

#include <stdio.h>


/**
** \brief initializes a stream according to a command line and a context
** \param context the context associated with the stream
** \param cs the pointer to store the result in
** \param arg_cont the command line to read from
** \return the status code of the operation.
**    if non zero, the program shall exit
*/
int cstream_dispatch_init(struct repl *context, struct cstream **cs, struct cli_options *arg_cont);
