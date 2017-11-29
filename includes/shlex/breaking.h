#pragma once

#include "io/cstream.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


bool is_breaking(char c);
void read_breaking(s_cstream *cs, s_token *tok);
