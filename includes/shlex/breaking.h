#pragma once

#include "io/cstream.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


/**
** \brief whether a char should break the token being built
*/
bool is_breaking(char c);

/**
** \brief read a word starting with a breaking character
*/
void read_breaking(s_cstream *cs, s_token *tok);
