#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"

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
int if_exec(s_env *env, struct ast *ast, s_errcont *cont);
void if_free(struct ast *ast);
