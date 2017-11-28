#pragma once

#include "wordlist.h"


typedef struct afor
{
  s_wordlist *var;
  s_wordlist *collection;
  struct ast *actions;
} s_afor;


#define AFOR(Var, Collection, Actions)          \
  ((s_afor)                                     \
  {                                             \
    (Var), (Collection), (Actions)              \
  })
