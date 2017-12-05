#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "io/cstream.h"
#include "utils/error.h"
#include "utils/evect.h"


#define TOK_IS_OP(Type) ((Type) >= TOK_LESS && (Type) <= TOK_LESSDASH)
#define LEX_OP_TOKS(F)    \
  F(TOK_LESS,      "<")   \
  F(TOK_GREAT,     ">")   \
  F(TOK_SEMI,      ";")   \
  F(TOK_AND,       "&")   \
  F(TOK_PIPE,      "|")   \
                          \
  F(TOK_LPAR,   "(")      \
  F(TOK_RPAR,   ")")      \
                          \
  F(TOK_AND_IF,    "&&")  \
  F(TOK_OR_IF,     "||")  \
  F(TOK_DSEMI,     ";;")  \
  F(TOK_DLESS,     "<<")  \
  F(TOK_DGREAT,    ">>")  \
  F(TOK_LESSAND,   "<&")  \
  F(TOK_GREATAND,  ">&")  \
  F(TOK_LESSGREAT, "<>")  \
  F(TOK_CLOBBER,   ">|")  \
  F(TOK_LESSDASH,  "<<-") \


#define TOK_IS_KW(Type) ((Type) >= TOK_IF && (Type) <= TOK_FUNC)
#define TOK_KW_ALIGN(Type) ((Type) - TOK_IF)

#define LEX_KW_TOKS(F)   \
  F(TOK_IF,     "if")    \
  F(TOK_THEN,   "then")  \
  F(TOK_ELSE,   "else")  \
  F(TOK_ELIF,   "elif")  \
  F(TOK_FI,     "fi")    \
  F(TOK_DO,     "do")    \
  F(TOK_DONE,   "done")  \
                         \
  F(TOK_CASE,   "case")  \
  F(TOK_ESAC,   "esac")  \
  F(TOK_WHILE,  "while") \
  F(TOK_UNTIL,  "until") \
  F(TOK_FOR,    "for")   \
  F(TOK_LBRACE, "{")     \
  F(TOK_RBRACE, "}")     \
  F(TOK_BANG,   "!")     \
  F(TOK_IN,     "in")    \
                         \
  F(TOK_FUNC  , "function")


#define LEX_CONST_ENUM(TokName, Value) TokName,

#define TOK_IS_GEN_DET(Type)                    \
  ((Type) == TOK_NEWLINE || (Type) == TOK_EOF || (Type) == TOK_IO_NUMBER)
#define LEX_GEN_TOKS(F)   \
  F(TOK_WORD)             \
  F(TOK_ASSIGNMENT_WORD)  \
  F(TOK_NAME)             \
  F(TOK_NEWLINE)          \
  F(TOK_IO_NUMBER)        \
                          \
  F(TOK_EOF)

#define TOK_IS_DET(Type) (TOK_IS_GEN_DET(Type) || TOK_IS_OP(Type))

#define LEX_GEN_TOKS_ENUM(Tokname) Tokname,

#define TOK_BUF_MIN_SIZE 10

typedef struct token
{
  enum token_type
  {
    LEX_OP_TOKS(LEX_CONST_ENUM)
    LEX_KW_TOKS(LEX_CONST_ENUM)
    LEX_GEN_TOKS(LEX_GEN_TOKS_ENUM)
  } type;

  s_lineinfo lineinfo;
  int delim;
  bool specified;
  struct evect str;
  struct token *next;
} s_token;


#define TOK_PUSH(Tok, C) evect_push(&(Tok)->str, C)
#define TOK_SIZE(Tok) (Tok)->str.size
#define TOK_STR(Tok) ((Tok)->str.data)

typedef struct lexer
{
  s_token *head;
  s_cstream *stream;
} s_lexer;

s_token *tok_alloc(s_lexer *lexer);
void tok_free(s_token *free, bool free_buf);

bool tok_is(const s_token *tok, enum token_type type);

s_lexer *lexer_create(s_cstream *stream);
void lexer_free(s_lexer *lexer);

const s_token *lexer_peek(s_lexer *lexer, s_errman *errman);
s_token *lexer_pop(s_lexer *lexer, s_errman *errman);
void lexer_push(s_lexer *lexer, s_token *tok);

void word_read(s_cstream *cs, s_token *tok, s_errman *errman);
