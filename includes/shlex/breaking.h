#pragma once

#include "io/cstream.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


/**
** \brief whether a char should break the token being built
** \param c the considered character
*/
bool is_breaking(char c);

/**
** \brief read a word starting with a breaking character
** \param cs the stream to read from
** \param the token to read into
*/
void read_breaking(s_cstream *cs, s_token *tok);
