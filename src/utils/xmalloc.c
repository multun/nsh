#include "utils/alloc.h"

#include <err.h>
#include <stdlib.h>


void *xmalloc(size_t size)
{
  void *ret = malloc(size);
  if (size && !ret)
    err(1, "malloc failed");
  return ret;
}
