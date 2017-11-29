#pragma once

#include "io/cstream.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


bool is_breaking(char c, size_t pos);
bool read_breaking(s_cstream *cs, s_token *tok, s_sherror **error);
