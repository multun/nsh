#pragma once

#include <stdio.h>

#include "../io/cstream.h"



typedef struct token
{
  enum token_type
  {
    TOK_WORD,
    TOK_ASSIGNEMENT_WORD,
    TOK_NAME,
    TOK_NEWLINE,
    TOK_IO_NUMBER,
    TOK_EOF,
  } type;

  union
  {
    int io_number;
    struct dbuf str;
  } data;
} s_token;


#define TOK_STR(Tok) ((Tok)->data.str.buf1)

typedef struct lexer
{
  s_token *peek;
  s_cstream *stream;
} s_lexer;


s_lexer *lexer_create(s_cstream *stream);
void lexer_free(s_lexer *lexer);
