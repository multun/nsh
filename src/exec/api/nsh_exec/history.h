#pragma once

#include <nsh_exec/repl.h>

#include <stdio.h>

/**
** \brief initializes the history context
*/
void history_init(struct repl *ctx, FILE *history_file);

/**
** \brief updates the history
** \details if there is a readline stream running, read its line buffer and
**   flush it into the history file. Also update the readline internal history.
*/
void history_update(struct repl *ctx);

/**
** \brief close the history file if opened
*/
void history_destroy(struct repl *ctx);
