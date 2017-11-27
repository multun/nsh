#include <stdlib.h>

#include "lexer.h"


lexer_create(s_stream *stream)
{
  s_lexer *lex = malloc(sizeof (s_lexer));
  lex->stream = stream;
  lex->peek = NULL;
}

lexer_free(s_lexer *lexer)
{
  free(lexer);
}
