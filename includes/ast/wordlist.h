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


char **wordlist_to_argv(s_wordlist *wl, s_env *env);
void wordlist_free(s_wordlist *wl, bool free_buf);
