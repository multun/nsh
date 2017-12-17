#pragma once

#include <stddef.h>

/**
** \brief information about the current context
** \desc used for error messages
*/
typedef struct lineinfo
{
  /**
  ** the current line number
  */
  size_t line;
  /**
  ** the current column number
  */
  size_t column;
  /**
  ** the whole source code
  */
  const char *source;
} s_lineinfo;


#define LINEINFO(Source) \
  (s_lineinfo)           \
  {                      \
    .line = 1,           \
    .column = 1,         \
    .source = (Source),  \
  }
