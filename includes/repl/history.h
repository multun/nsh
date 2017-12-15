#pragma once

#include "repl/repl.h"

#include <stdio.h>


FILE *history_open(void);
void history_init(s_context *ctx);
void history_update(s_context *ctx);
void history_destroy(s_context *ctx);
