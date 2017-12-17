#pragma once

#include "utils/error.h"

/**
** \brief removes the quotes from a string variable
** \details on exception, the caller should free *sres
** \param sres an address to store the result at
** \param str the string to unquote
** \param cont the error context
*/
void unquote(char **sres, const char *str, s_errcont *cont);
