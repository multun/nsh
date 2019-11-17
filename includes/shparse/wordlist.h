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

/**
** \brief represents a linked list of word
*/
struct wordlist
{
    struct pvect words;
};

static inline void wordlist_init(struct wordlist *wl)
{
    pvect_init(&wl->words, 4);
}

static inline void wordlist_push(struct wordlist *wl, struct shword *word)
{
    pvect_push(&wl->words, word);
}

static inline size_t wordlist_size(struct wordlist *wl)
{
    return pvect_size(&wl->words);
}

static inline struct shword *wordlist_get(struct wordlist *wl, size_t i)
{
    return pvect_get(&wl->words, i);
}

static inline char *wordlist_get_str(struct wordlist *wl, size_t i)
{
    return shword_buf(wordlist_get(wl, i));
}

static inline void wordlist_destroy(struct wordlist *wl)
{
    for (size_t i = 0; i < wordlist_size(wl); i++)
        free(wordlist_get(wl, i));
    pvect_destroy(&wl->words);
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
