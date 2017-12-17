#pragma once

#include "utils/error.h"
#include "shexec/environment.h"

#include <stdbool.h>


#define DECLARE_ARTH_TYPE_ENUM(Name, Parser, Exec)                            \
  Name,

#define DECLARE_ARTH_PARSER_UTILS(Name, Parser, Exec)                         \
  Parser,

#define DECLARE_ARTH_EXEC_UTILS(Name, Parser, Exec)                           \
  Exec,

#define ARTH_TYPE_APPLY(F)                                                    \
  F(ARTH_OR, arth_parse_or, arth_exec_or)                                     \
  F(ARTH_AND, arth_parse_and, arth_exec_and)                                  \
  F(ARTH_BOR, arth_parse_bor, arth_exec_bor)                                  \
  F(ARTH_XOR, arth_parse_xor, arth_exec_xor)                                  \
  F(ARTH_BAND, arth_parse_band, arth_exec_band)                               \
  F(ARTH_MINUS, arth_parse_plus, arth_exec_minus)                             \
  F(ARTH_PLUS, arth_parse_plus, arth_exec_plus)                               \
  F(ARTH_DIV, arth_parse_time, arth_exec_div)                                 \
  F(ARTH_TIME, arth_parse_time, arth_exec_time)                               \
  F(ARTH_POW, arth_parse_pow, arth_exec_pow)                                  \
  F(ARTH_NOT, arth_parse_not, arth_exec_not)                                  \
  F(ARTH_BNOT, arth_parse_bnot, arth_exec_bnot)                               \
  F(ARTH_WORD, arth_parse_word, arth_exec_and)


typedef struct arth_ast
{
  enum
  {
    ARTH_TYPE_APPLY(DECLARE_ARTH_TYPE_ENUM)
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

s_arth_ast *arth_parse_rec(char **start, char **end,
                           s_env *env, s_errcont *cont);

void arth_default_index(int *index, char **elms);
bool is_arth_op(char c);

char **arth_lex(char *str, char ***end);
void arth_free(s_arth_ast *ast);

int arth_exec(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_or(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_and(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_bor(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_xor(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_band(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_plus(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_minus(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_div(s_arth_ast *ast, s_env *env, s_errcont *cont);
int arth_exec_time(s_arth_ast *ast, s_env *env, s_errcont *cont);

int arth_exec_pow(s_arth_ast *ast, s_env *env, s_errcont *cont);

int arth_exec_not(s_arth_ast *ast, s_env *env, s_errcont *cont);

int arth_exec_bnot(s_arth_ast *ast, s_env *env, s_errcont *cont);


s_arth_ast *arth_parse(char *str, s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_word(char **str, char **end,
                            s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_or(char **start, char **end,
                          s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_and(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_bor(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_xor(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_band(char **start, char **end,
                            s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_plus(char **start, char **end,
                            s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_minus(char **start, char **end,
                             s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_div(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_time(char **start, char **end,
                            s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_pow(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_not(char **start, char **end,
                           s_env *env, s_errcont *cont);

s_arth_ast *arth_parse_bnot(char **start, char **end,
                            s_env *env, s_errcont *cont);
