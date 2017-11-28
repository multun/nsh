#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <stdlib.h>


s_lexer *lexer_create(s_cstream *stream)
{
  s_lexer *ret = xmalloc(sizeof(*ret));
  ret->stream = stream;
  ret->head = NULL;
  return ret;
}


void lexer_free(s_lexer *lexer)
{
  free(lexer);
  // TODO: free other fields
}


static s_token *lexer_lex(s_lexer *lexer)
{
  (void)lexer;
  return NULL;
}


const s_token *lexer_peek(s_lexer *lexer)
{
  if (!lexer->head)
    lexer->head = lexer_lex(lexer);
  return lexer->head;
}


s_token *lexer_pop(s_lexer *lexer)
{
  if (lexer->head)
  {
    s_token *ret = lexer->head;
    lexer->head = NULL;
    return ret;
  }
  return lexer_lex(lexer);
}
