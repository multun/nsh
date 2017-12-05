#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "shexec/environment.h"


typedef struct wordlist
{
  char *str;
  bool split;
  bool expand;
  struct wordlist *next;
} s_wordlist;


#define WORDLIST(Str, Split, Expand, Next)        \
  ((s_wordlist)                                   \
  {                                               \
    (Str), (Split), (Expand), (Next)              \
  })

#define WL(Str)                                   \
  WORDLIST(Str, false, false, NULL)


/**
** \brief generate from a wordlist a table of arguments
** \param wl the wordlist
** \param env the current environment
**/
char **wordlist_to_argv(s_wordlist *wl, s_env *env);

/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(s_wordlist *wl, bool free_buf);
