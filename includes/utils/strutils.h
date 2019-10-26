#pragma once

#include <stdlib.h>
#include <errno.h>
#include <limits.h>

/* log10(n) = log2(n) * (ln(2) / ln(10)) */
/* log10(n) <= log2(n) * 0.3 + 1 */
#define UINT_MAX_CHARS(IntegerType)                     \
    ((sizeof(IntegerType) * CHAR_BIT) / 3               \
     + 1 /* last digit (round to the upper decimal) */) \

#define INT_MAX_CHARS(IntegerType)                      \
    (1 /* - sign */ + UINT_MAX_CHARS(IntegerType))


static inline int parse_long(const char *str, long *val)
{
    char *temp;

    errno = 0;
    *val = strtol(str, &temp, 0);
    if (temp == str || *temp != '\0')
        return -1;

    if ((*val == LONG_MIN || *val == LONG_MAX) && errno == ERANGE)
        return -1;

    return 0;
}

static inline int parse_int(const char *str, int *val)
{
    long res;
    if (parse_long(str, &res) != 0)
        return -1;

    if (res > INT_MAX || res < INT_MIN)
    {
        errno = ERANGE;
        return -1;
    }

    *val = res;
    return 0;
}
