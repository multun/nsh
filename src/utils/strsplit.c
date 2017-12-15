#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "utils/strsplit.h"


static bool is_delim(const char **delim, size_t nb_delim, char *str)
{
  for (size_t i = 0; i < nb_delim; i++)
    if (!strncmp(delim[i], str, strlen(delim[i])))
      return true;
  return false;
}


size_t strsplit_r(char **str, const char **delim, size_t nb_delim)
{
  size_t res = 0;
  for (; str[res]; res++)
    if (is_delim(delim, nb_delim, str[res]))
      return res;
  return res;
}
