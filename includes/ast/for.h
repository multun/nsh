#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"


typedef struct afor
{
  s_wordlist *var;
  s_wordlist *collection;
  struct ast *actions;
} s_afor;


#define AFOR(Var, Collection, Actions)                            \
  ((s_afor)                                                       \
  {                                                               \
    (Var), (Collection), (Actions)                                \
  })

#define AST_AFOR(Var, Collection, Actions)                        \
  AST(SHNODE_FOR, for, AFOR(Var, Collection, Actions))


void for_print(FILE *f, struct ast *ast);
int for_exec(s_env *env, struct ast *ast, s_errcont *cont);
void for_free(struct ast *ast);
