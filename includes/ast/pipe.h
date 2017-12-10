#pragma once

#include "ast/ast.h"


/**
** \brief represents a pipe node
*/
typedef struct apipe
{
  struct ast *left;
  struct ast *right;
} s_apipe;


#define APIPE(Left, Right)                                        \
  ((s_apipe)                                                      \
  {                                                               \
    (Left), (Right)                                               \
  })

#define AST_PIPE(Left, Right)                                     \
  AST(SHNODE_PIPE, pipe, APIPE(Left, Right))


/**
** \brief print in dot format the representation of a pipe node
*/
void pipe_print(FILE *f, struct ast *ast);


/**
** \brief exec a pipe node
*/
void pipe_free(struct ast *ast);


/**
** \brief free a pipe node
*/
int pipe_exec(s_env *env, struct ast *ast, s_errcont *cont);
