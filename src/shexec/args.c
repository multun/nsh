#include "utils/alloc.h"

#include <stdlib.h>
#include <string.h>


char **argv_dup(char **argv)
{
  size_t size = 0;
  while (argv[size])
    size++;

  char **ret = xmalloc((size + 1) * sizeof(void *));
  for (size_t i = 0; i < size; i++)
    ret[i] = strdup(argv[i]);
  ret[size] = NULL;
  return ret;
}


void argv_free(char **argv)
{
  for (size_t i = 0; argv[i]; i++)
    free(argv[i]);
  free(argv);
}
