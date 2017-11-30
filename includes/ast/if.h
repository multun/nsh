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

#define AST_AIF(Condition, Success, Failure)  \
  AST(SHNODE_IF, if,                          \
      AIF(Condition, Success, Failure))


void if_print(FILE *f, struct ast *node);
