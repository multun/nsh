#pragma once

#include <nsh_exec/repl.h>


/**
** \brief parse command line options
** \return if negative, the parsing failed and the expected
**   return code is -(retcode + 1). otherwise, the index of
**   the first non-option argument is returned
*/
int parse_cli_options(struct cli_options *res, int argc, char *argv[]);
