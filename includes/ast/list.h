#pragma once

#include "ast/ast.h"
#include "utils/error.h"

typedef struct alist
{
  struct ast *action;
  struct alist *next;
} s_alist;


#define ALIST(Action, Next)                                       \
  ((s_alist)                                                      \
  {                                                               \
    (Action), (Next)                                              \
  })

#define AST_ALIST(Action, Next)                                   \
  AST(SHNODE_LIST, list, ALIST(Action, Next))


void list_print(FILE *f, struct ast *ast);
int list_exec(s_env *env, struct ast *ast, s_errcont *cont);
void list_free(struct ast *ast);
