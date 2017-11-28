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

const s_token *lexer_peek(s_lexer *lexer)
{
  (void)lexer;
  return NULL;
}

s_token *lexer_pop(s_lexer *lexer)
{
  (void)lexer;
  return NULL;
}
