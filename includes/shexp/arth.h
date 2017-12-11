#pragma once

#include <stdbool.h>

enum arth_ast_type
{
  ARTH_WORD,
  ARTH_PLUS,
  ARTH_MINUS,
  ARTH_NOT,
  ARTH_BNOT,
  ARTH_POW,
  ARTH_TIME,
  ARTH_DIVE,
  ARTH_BAND,
  ARTH_XOR,
  ARTH_BOR,
  ARTH_AND,
  ARTH_OR,
};


typedef struct arth_ast
{
  enum arth_ast_type type;
  struct arth_ast *left;
  struct arth_ast *right;
} s_arth_ast;


#define ARTH_AST(Type, Left, Right)                 \
((s_arth_ast)                                       \
{                                                   \
  .type = Type,                                     \
  .left = Left,                                     \
  .right = Right,                                   \
})

void arth_default_index(int &index, char **elms);
bool is_arth_op(char c);

char **arth_lex(char *str);
arth_ast *arth_parse(char *str, bool *err);
