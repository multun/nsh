#pragma once

#include "ast/ast.h"
#include "shexec/environment.h"
#include "utils/error.h"


/**
** \brief represents an unitl node
*/
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


/**
** \brief print in dot format the representation of an until node
*/
void until_print(FILE *f, struct ast *ast);


/**
** \brief exec an until node
*/
int until_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free an until node
*/
void until_free(struct ast *ast);
