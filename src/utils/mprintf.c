#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "utils/alloc.h"


char *mprintf(const char *fmt, ...)
{
    va_list ap;

    /* Determine required size */
    va_start(ap, fmt);
    int size = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (size < 0)
        return NULL;

    char *res = xmalloc(size + 1);
    if (res == NULL)
        return NULL;

    va_start(ap, fmt);
    size = vsprintf(res, fmt, ap);
    va_end(ap);

    if (size >= 0)
        return res;

    free(res);
    return NULL;
}
