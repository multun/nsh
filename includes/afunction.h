#pragma-once

#include "wordlist.h"


typedef struct afunction
{
  s_wordlist *name;
  struct ast *value;
} s_afunction;
