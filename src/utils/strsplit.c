#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "utils/strsplit.h"


char **strsplit_r(char **start, char **end,  const char *delim, bool first)
{
  size_t delim_len = strlen(delim);
  char **res = NULL;
  for (; start < end; start++)
    if (!strncmp(*start, delim, delim_len))
    {
      if (first)
        return start;
      res = start;
    }
  return res;
}
