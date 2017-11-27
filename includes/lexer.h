#pragma-once

#include <stdio.h>


typedef struct token
{
  enum token_type
  {
    WORD,
    ASSIGNEMENT_WORD,
    NAME,
    NEWLINE,
    IO_NUMBER,
    END_OF_FILE,
  } type;
  s_buf *buf;
} s_token;


typedef struct lexer
{
  s_token *peek;
  s_stream *stream;
} s_lexer;


lexer_create(s_stream *stream);
lexer_free(s_lexer *lexer);
