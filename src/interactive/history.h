#pragma once

#include <nsh_exec/environment.h>


BUILTINS_DECLARE(history)

/** \brief Opens the history file */
FILE *history_open(void);
