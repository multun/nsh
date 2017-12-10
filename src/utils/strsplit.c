#include <stddef.h>
#include <stdbool.h>
#include <string.h>

static size_t is_delim(const char **delim, size_t nb_delim, char *c)
{
  for (size_t i = 0; i < nb_delim; i++)
    if (!strncmp(delim[i], c, strlen(delim[i])))
      return strlen(delim[i]);
  return 0;
}


static char *strsplit_r_sub(char **str, const char **delim, size_t nb_delim)
{
  char *beg = *str;
  size_t i = 0;
  while (!(i = is_delim(delim, nb_delim, *str)))
    if (!**str)
    {
      *str = NULL;
      return beg;
    }
    else
      (*str)++;

  if (!**str)
    return NULL;
  **str = '\0';
  (*str) += i;
  return beg;
}


char* strsplit_r(char *str, const char **delim,
                 size_t nb_delim, char **saveptr)
{
  if (str)
    *saveptr = str;

  char *ret;

  do {
    if (!delim || !saveptr || !*saveptr || !**saveptr)
      return NULL;
  } while ((ret = strsplit_r_sub(saveptr, delim, nb_delim)) && *ret == '\0');
  return ret;
}
