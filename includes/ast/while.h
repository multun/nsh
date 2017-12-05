#pragma once

#include "ast/ast.h"
#include "shexec/environment.h"


typedef struct awhile
{
  struct ast *condition;
  struct ast *actions;
} s_awhile;


#define AWHILE(Condition, Actions)                        \
  ((s_awhile)                                             \
  {                                                       \
    (Condition), (Actions)                                \
  })

#define AST_AWHILE(Condition, Actions)                    \
  AST(SHNODE_WHILE, while, AWHILE(Condition, Actions))


void while_print(FILE *f, struct ast *ast);
int while_exec(s_env *env, struct ast *ast);
void while_free(struct ast *ast);
