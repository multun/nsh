#pragma once

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


/* gives the number of characters needed to print some type in base 8 or 10.
** log10(n) = log2(n) * (ln(2) / ln(10))
** log8(n) = log2(n) * (ln(2) / ln(8))
** log10(n) <= log2(n) / 3 + 1
** log8(n) <= log2(n) / 3 + 1
*/
#define UINT_MAX_CHARS(IntegerType)                                                      \
    ((sizeof(IntegerType) * CHAR_BIT) / 3                                                \
     + 1 /* last digit (round to the upper decimal) */)

#define INT_MAX_CHARS(IntegerType) (1 /* - sign */ + UINT_MAX_CHARS(IntegerType))


static inline bool startswith(const char *str, const char *prefix)
{
    size_t prefix_length = strlen(prefix);
    return strncmp(str, prefix, prefix_length) == 0;
}

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

    if (res > INT_MAX || res < INT_MIN) {
        errno = ERANGE;
        return -1;
    }

    *val = res;
    return 0;
}


static inline bool portable_filename_char(char c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')
            || (c == '.') || (c == '_') || (c == '-'));
}
