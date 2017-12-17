#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "utils/strsplit.h"


char **strsplit_r(char **start, char **end,  const char **delims, bool first)
{
  char **res = NULL;
  for (; start < end; start++)
    for (const char **delim = delims; *delim; delim++)
      if (!strcmp(*start, *delim))
      {
        if (first)
          return start;
        res = start;
      }
  return res;
}
