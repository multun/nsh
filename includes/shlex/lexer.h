#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "io/cstream.h"
#include "utils/error.h"
#include "utils/evect.h"


#define LEX_OPS(F)        \
  F(TOK_AND_IF,    "&&")  \
  F(TOK_OR_IF,     "||")  \
  F(TOK_DSEMI,     ";;")  \
  F(TOK_DLESS,     "<<")  \
  F(TOK_DGREAT,    ">>")  \
  F(TOK_LESSAND,   "<&")  \
  F(TOK_GREATAND,  ">&")  \
  F(TOK_LESSGREAT, "<>")  \
  F(TOK_LESSDASH,  "<<-") \
                          \
  F(TOK_CLOBBER,   ">|")  \
  F(TOK_LESS,      "<")   \
  F(TOK_GREAT,     ">")   \
                          \
  F(TOK_SEMI,      ";")   \
  F(TOK_AND,       "&")   \
  F(TOK_PIPE,      "|")

#define LEX_OPS_ENUM(TokName, Value) TokName,


#define LEX_GEN_TOKS(F)   \
  F(TOK_WORD)             \
  F(TOK_ASSIGNEMENT_WORD) \
  F(TOK_NAME)             \
  F(TOK_NEWLINE)          \
  F(TOK_IO_NUMBER)        \
                          \
  F(TOK_IF)               \
  F(TOK_THEN)             \
  F(TOK_ELSE)             \
  F(TOK_ELIF)             \
  F(TOK_FI)               \
  F(TOK_DO)               \
  F(TOK_DONE)             \
                          \
  F(TOK_CASE)             \
  F(TOK_ESAC)             \
  F(TOK_WHILE)            \
  F(TOK_UNTIL)            \
  F(TOK_FOR)              \
  F(TOK_LPAR)             \
  F(TOK_RPAR)             \
  F(TOK_LBRACE)           \
  F(TOK_RBRACE)           \
  F(TOK_BANG)             \
  F(TOK_IN)               \
                          \
  F(TOK_FUNC)             \
                          \
  F(TOK_EOF)

#define LEX_GEN_TOKS_ENUM(Tokname) Tokname,

#define TOK_BUF_MIN_SIZE 10

typedef struct token
{
  enum token_type
  {
    LEX_OPS(LEX_OPS_ENUM)
    LEX_GEN_TOKS(LEX_GEN_TOKS_ENUM)
  } type;

  int delim;
  bool specified;
  struct evect str;
} s_token;


#define TOK_PUSH(Tok, C) evect_push(&(Tok)->str, C)
#define TOK_SIZE(Tok) (Tok)->str.size
#define TOK_STR(Tok) ((Tok)->str.data)

typedef struct lexer
{
  s_token *head;
  s_cstream *stream;
} s_lexer;

s_token *tok_alloc(void);
void tok_free(s_token *free, bool free_buf);

bool tok_is(const s_token *tok, enum token_type type);
s_token *tok_as(s_token *tok, enum token_type type);

s_lexer *lexer_create(s_cstream *stream);
void lexer_free(s_lexer *lexer);

const s_token *lexer_peek(s_lexer *lexer);
s_token *lexer_pop(s_lexer *lexer);

bool word_read(s_cstream *cs, s_token *tok, s_sherror **error);
