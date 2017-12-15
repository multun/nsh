#pragma once

#include <stdbool.h>



typedef struct arth_ast
{
  enum
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
  } type;
  struct arth_ast *left;
  struct arth_ast *right;
  int value;
} s_arth_ast;


#define ARTH_AST(Type, Left, Right)                 \
((s_arth_ast)                                       \
{                                                   \
  .type = (Type),                                   \
  .left = (Left),                                   \
  .right = (Right),                                 \
})

s_arth_ast *arth_parse_rec(char **start, char **end, bool *err);

void arth_default_index(int *index, char **elms);
bool is_arth_op(char c);

char **arth_lex(char *str, char ***end);
s_arth_ast *arth_parse(char *str, bool *err);
s_arth_ast *arth_parse_word(char **str, bool *err);
int arth_exec(s_arth_ast *ast);
