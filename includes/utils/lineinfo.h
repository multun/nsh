#pragma once

#include <stddef.h>


typedef struct lineinfo
{
  size_t line;
  size_t column;
  const char *source;
} s_lineinfo;


#define LINEINFO(Source) \
  (s_lineinfo)           \
  {                      \
    .line = 1,           \
    .column = 1,         \
    .source = (Source),  \
  }
