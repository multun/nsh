#pragma once

#include <stdbool.h>
#include <stddef.h>

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
