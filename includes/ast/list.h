#pragma once

#include "ast/ast.h"
#include "utils/error.h"


/**
** \brief represents a linked list of ast
*/
typedef struct alist
{
  struct ast *action; /**< the current ast */
  struct alist *next; /**< the next ast */
} s_alist;


#define ALIST(Action, Next)                                       \
  ((s_alist)                                                      \
  {                                                               \
    (Action), (Next)                                              \
  })

#define AST_ALIST(Action, Next)                                   \
  AST(SHNODE_LIST, list, ALIST(Action, Next))


/**
** \brief print in dot format the representation of a list node
*/
void list_print(FILE *f, struct ast *ast);


/**
** \brief exec a list node
*/
int list_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free a list node
*/
void list_free(struct ast *ast);
