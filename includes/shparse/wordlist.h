#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "shexec/environment.h"
#include "utils/error.h"
#include "utils/pvect.h"
#include "utils/cpvect.h"

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
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE
#undef GVECT_INIT_SIZE
#undef GVECT_DESTROY_ITEM

static inline char *wordlist_get_str(struct wordlist *wl, size_t i)
{
    return shword_buf(wordlist_get(wl, i));
}


/**
** \brief generate from a wordlist a table of arguments
** \param wl the wordlist
** \param env the current environment
** \return argc
**/
void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                     struct environment *env, struct errcont *cont);

/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(struct wordlist *wl, bool free_buf);
