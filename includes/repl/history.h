#pragma once

#include "repl/repl.h"

#include <stdio.h>

/**
** \brief opens the history file
** \return a handler to the history file
*/
FILE *history_open(void);

/**
** \brief initializes the history context
** \details depending on the context, it may either initialize the history
**   handler or do nothing
*/
void history_init(struct context *ctx);

/**
** \brief updates the history
** \details if there is a readline stream running, read its line buffer and
**   flush it into the history file. Also update the readline internal history.
*/
void history_update(struct context *ctx);

/**
** \brief close the history file if opened
*/
void history_destroy(struct context *ctx);
