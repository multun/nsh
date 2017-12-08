#pragma once

#include "ast/ast.h"
#include "shexec/environment.h"
#include "utils/error.h"


typedef struct auntil
{
  struct ast *condition;
  struct ast *actions;
} s_auntil;


#define AUNTIL(Condition, Actions)                                \
  ((s_auntil)                                                     \
  {                                                               \
    (Condition), (Actions)                                        \
  })

#define AST_AUNTIL(Condition, Actions)                            \
  AST(SHNODE_UNTIL, until, AUNTIL(Condition, Actions))


void until_print(FILE *f, struct ast *ast);
int until_exec(s_env *env, struct ast *ast, s_errcont *cont);
void until_free(struct ast *ast);
