#pragma once

#include "utils/error.h"
#include "shexec/environment.h"

#include <stdbool.h>

typedef struct arthcont
{
  s_env *env;
  s_errcont *cont;
} s_arthcont;


#define ARTHCONT(Env, Cont)                     \
  (s_arthcont)                                  \
  {                                             \
    .env = (Env),                               \
    .cont = (Cont),                             \
  }


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


/**
** \brief lex an arithmetic expansion
**
** \param str the raw expansion
** \param end a pointer to the last token
*/
char **arth_lex(char *str, char ***end);

/**
** \brief free an arithmetic ast
**
** \param ast ast to free
*/
void arth_free(s_arth_ast *ast);

/**
** \brief execute an arithmetic ast
**
** \param ast ast to execute
** \param cont the error context and environment
*/
int arth_exec(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an OR node of an arithmetic ast
**
** \param ast current OR ast node
** \param cont the error context and environment
*/
int arth_exec_or(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an AND node of an arithmetic ast
**
** \param ast current AND ast node
** \param cont the error context and environment
*/
int arth_exec_and(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an BOR node of an arithmetic ast
**
** \param ast current BOR ast node
** \param cont the error context and environment
*/
int arth_exec_bor(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an XOR node of an arithmetic ast
**
** \param ast current XOR ast node
** \param cont the error context and environment
*/
int arth_exec_xor(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an BAND node of an arithmetic ast
**
** \param ast current BAND ast node
** \param cont the error context and environment
*/
int arth_exec_band(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an PLUS node of an arithmetic ast
**
** \param ast current PLUS ast node
** \param cont the error context and environment
*/
int arth_exec_plus(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an MINUS node of an arithmetic ast
**
** \param ast current MINUS ast node
** \param cont the error context and environment
*/
int arth_exec_minus(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an DIV node of an arithmetic ast
**
** \param ast current DIV ast node
** \param cont the error context and environment
*/
int arth_exec_div(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an TIME node of an arithmetic ast
**
** \param ast current TIME ast node
** \param cont the error context and environment
*/
int arth_exec_time(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an POW node of an arithmetic ast
**
** \param ast current POW ast node
** \param cont the error context and environment
*/
int arth_exec_pow(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an NOT node of an arithmetic ast
**
** \param ast current NOT ast node
** \param cont the error context and environment
*/
int arth_exec_not(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief execute an BNOT node of an arithmetic ast
**
** \param ast current BNOT ast node
** \param cont the error context and environment
*/
int arth_exec_bnot(s_arth_ast *ast, s_arthcont *cont);

/**
** \brief parse an arithmetic expression
**
** \param str the raw expression
** \param cont the error context and environment
*/
s_arth_ast *arth_parse(char *str, s_arthcont *cont);

/**
** \brief parse recursively an arithmetic ast
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_rec(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a WORD ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_word(char **str, char **end,
                     s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse an OR ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_or(char **start, char **end,
                   s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse an AND ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_and(char **start, char **end,
                   s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a BOR ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_bor(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a XOR ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_xor(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a BAND ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_band(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a PLUS and MINUS ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_plus(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a TIME ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_time(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a POW ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_pow(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a NOT ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_not(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast);

/**
** \brief parse a BNOT ast node
**
** \param start the first token to parse
** \param end the last token to parse
** \param cont the error context and environment
** \param ast a pointer to the created node
*/
void arth_parse_bnot(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast);
