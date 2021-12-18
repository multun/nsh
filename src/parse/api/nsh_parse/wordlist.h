#pragma once

#include <nsh_utils/lineinfo.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include <nsh_utils/pvect.h>
#include <nsh_utils/cpvect.h>

struct shword
{
    struct lineinfo line_info;
    char buf[];
};

static inline char *shword_buf(struct shword *word)
{
    return word->buf;
}

static inline struct lineinfo *shword_line_info(struct shword *word)
{
    return &word->line_info;
}

#define GVECT_DESTROY_ITEM free
#define GVECT_INIT_SIZE 10
#define GVECT_NAME wordlist
#define GVECT_TYPE struct shword *
#include <nsh_utils/pvect_wrap.h>
#undef GVECT_NAME
#undef GVECT_TYPE
#undef GVECT_INIT_SIZE
#undef GVECT_DESTROY_ITEM

static inline char *wordlist_get_str(struct wordlist *wl, size_t i)
{
    return shword_buf(wordlist_get(wl, i));
}

/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(struct wordlist *wl, bool free_buf);
