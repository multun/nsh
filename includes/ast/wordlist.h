#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "shexec/environment.h"
#include "utils/error.h"


/**
** \brief represents a wordlist
*/
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
void wordlist_to_argv(char ***res, s_wordlist *wl, s_env *env, s_errcont *cont);


/**
** \brief free a wordlist
** \param wl the wordlist
**/
void wordlist_free(s_wordlist *wl, bool free_buf);
