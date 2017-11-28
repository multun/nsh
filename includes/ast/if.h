#pragma once

#include "wordlist.h"


typedef struct aif
{
  struct ast *condition;
  struct ast *success;
  struct ast *failure;
} s_aif;


#define AIF(Condition, Success, Failure)      \
  ((s_aif)                                    \
  {                                           \
    (Condition), (Success), (Failure)         \
  })
