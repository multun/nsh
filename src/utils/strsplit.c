#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "utils/strsplit.h"


char **strsplit_r(char **start, char **end,  const char *delim, bool first)
{
  char **res = NULL;
  for (; start < end; start++)
    if (!strcmp(*start, delim))
    {
      if (first)
        return start;
      res = start;
    }
  return res;
}
