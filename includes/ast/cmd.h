#pragma once

#include "wordlist.h"


typedef struct acmd
{
  s_wordlist *wordlist;
} s_acmd;


#define ACMD(Wordlist)                          \
  ((s_acmd)                                     \
  {                                             \
    (Wordlist)                                  \
  })
