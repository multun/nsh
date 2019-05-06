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

void *xrealloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (size && !ret)
        err(1, "realloc failed");
    return ret;
}

void *xcalloc(size_t nmemb, size_t size)
{
    void *ret = calloc(nmemb, size);
    if (size && nmemb && !ret)
        err(1, "calloc failed");
    return ret;
}
