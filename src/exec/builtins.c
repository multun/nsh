#include <nsh_exec/environment.h>
#include <nsh_utils/macros.h>

#include <stddef.h>
#include <string.h>

#define BUILTINS_TAB(Name) {#Name, builtin_##Name},

static const struct
{
    const char *name;
    f_builtin func;
} g_builtins_tab[] = {BUILTINS_APPLY(BUILTINS_TAB)};

f_builtin find_default_builtin(const char *name)
{
    for (size_t i = 0; i < ARR_SIZE(g_builtins_tab); i++)
        if (!strcmp(name, g_builtins_tab[i].name))
            return g_builtins_tab[i].func;
    return NULL;
}
