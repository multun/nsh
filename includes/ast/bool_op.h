#pragma once


typedef struct abool_op
{
  enum bool_type
  {
    BOOL_OR,
    BOOL_AND,
    BOOL_NOT,
  } type;
  struct ast *left;
  struct ast *right;
} s_abool_op;


#define ABOOL_OP(Type, Left, Right)                               \
  ((s_abool_op)                                                   \
  {                                                               \
    (Type), (Left), (Right)                                       \
  })

#define AST_ABOOL_OP(Type, Left, Right)                           \
  AST(SHNODE_BOOL_OP, bool_op, ABOOL_OP(Type, Left, Right))


void bool_op_print(FILE *f, struct ast *ast);
int bool_op_exec(struct ast *ast);
