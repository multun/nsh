#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "shexec/environment.h"
#include "utils/error.h"

/**
** \brief represents a linked list of word
*/
struct wordlist
{
    char *str; /**< the current word string */
    bool split; /**< if the word has to be split */
    bool expand; /**< if the word has to be expand */
    struct wordlist *next; /**< the next word */
};

#define WORDLIST(Str, Split, Expand, Next)                                               \
    ((struct wordlist){ .str = (Str), .split = (Split), .expand = (Expand), .next = (Next)})

#define WL(Str) WORDLIST(Str, false, false, NULL)

/**
** \brief generate from a wordlist a table of arguments
** \param wl the wordlist
** \param env the current environment
** \return argc
**/
int wordlist_to_argv(char ***res, struct wordlist *wl, struct environment*env, struct errcont *cont);

/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(struct wordlist *wl, bool free_buf);
