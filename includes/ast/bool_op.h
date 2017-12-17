#pragma once

#include "shexec/environment.h"
#include "utils/error.h"


/**
** \brief represents a bool operator node
*/
typedef struct abool_op
{
  enum bool_type
  {
    BOOL_OR,
    BOOL_AND,
    BOOL_NOT,
  } type; /**< the type of operator */
  struct ast *left; /**< the first operand */
  struct ast *right; /**< the second operand */
} s_abool_op;


#define ABOOL_OP(Type, Left, Right)                               \
  ((s_abool_op)                                                   \
  {                                                               \
    (Type), (Left), (Right)                                       \
  })

#define AST_ABOOL_OP(Type, Left, Right)                           \
  AST(SHNODE_BOOL_OP, bool_op, ABOOL_OP(Type, Left, Right))


/**
** \brief print in dot format the representation of a bool operator node
*/
void bool_op_print(FILE *f, struct ast *ast);


/**
** \brief exec a bool operator node
*/
int bool_op_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free a bool operator node
*/
void bool_op_free(struct ast *ast);
