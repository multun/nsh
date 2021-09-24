#include "shexec/shopt.h"

#include <stddef.h>
#include <string.h>


#define STRING_LIST(Enum, StrRepr) StrRepr,


static const char *g_shopt_tab[SHOPT_COUNT] = {SHOPTS_APPLY(STRING_LIST)};


enum shopt shopt_from_string(const char *str)
{
    for (size_t i = 0; i < SHOPT_COUNT; i++)
        if (!strcmp(str, g_shopt_tab[i]))
            return i;
    return SHOPT_COUNT;
}

const char *string_from_shopt(size_t index)
{
    if (index >= SHOPT_COUNT)
        return NULL;
    return g_shopt_tab[index];
}
