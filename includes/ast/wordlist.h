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
    struct wordlist *next; /**< the next word */
};

#define WORDLIST(Str, Next)                                               \
    ((struct wordlist){ .str = (Str), .next = (Next)})

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
