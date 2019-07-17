#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "shexec/environment.h"
#include "utils/error.h"

/**
** \brief represents a linked list of word
*/
typedef struct wordlist
{
    char *str; /**< the current word string */
    bool split; /**< if the word has to be split */
    bool expand; /**< if the word has to be expand */
    struct wordlist *next; /**< the next word */
} s_wordlist;

#define WORDLIST(Str, Split, Expand, Next)                                               \
    ((s_wordlist){ .str = (Str), .split = (Split), .expand = (Expand), .next = (Next)})

#define WL(Str) WORDLIST(Str, false, false, NULL)

/**
** \brief generate from a wordlist a table of arguments
** \param wl the wordlist
** \param env the current environment
** \return argc
**/
int wordlist_to_argv(char ***res, s_wordlist *wl, s_env *env, s_errcont *cont);

/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(s_wordlist *wl, bool free_buf);
