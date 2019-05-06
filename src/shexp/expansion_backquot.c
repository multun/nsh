#include "shexp/expansion.h"
#include "utils/evect.h"

#include <string.h>
#include <assert.h>

static char *subshell_find_backquote(char *str)
{
    while (*str && *str != '`') {
        if (*str == '\\')
            str++;
        if (*str)
            str++;
    }
    assert(*str == '`');
    return str;
}

static void expand_sub_backquote(s_errcont *cont, char **str, s_env *env, s_evect *vec)
{
    char *buf = strndup(*str, subshell_find_backquote(*str) - *str);
    expand_subshell_buffer(cont, buf, env, vec);
    *str = subshell_find_backquote(*str) + 1;
}

bool expand_backquote(s_errcont *cont, s_exp_ctx ctx, s_env *env, s_evect *vec)
{
    if (**ctx.str != '`')
        return false;
    (*ctx.str)++;
    if (!ctx.quoted)
        evect_push(vec, '"');

    expand_sub_backquote(cont, ctx.str, env, vec);

    if (!ctx.quoted)
        evect_push(vec, '"');
    return true;
}
