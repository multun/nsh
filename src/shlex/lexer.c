#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <assert.h>
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
}


static s_token *lexer_lex(s_lexer *lexer, s_sherror **error)
{
  s_token *res = tok_alloc();
  word_read(lexer->stream, res, error);

  // TODO: if the word is a single carriage
  // return, this function should detect it
  if (!res->str.size)
  {
    tok_free(res, true);
    return NULL;
  }

  TOK_PUSH(res, '\0');
  return res;
}


const s_token *lexer_peek(s_lexer *lexer)
{
  s_sherror *error = NULL; // TODO: removeme
  if (!lexer->head)
    lexer->head = lexer_lex(lexer, &error);
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
  s_sherror *error = NULL;  // TODO: removeme
  return lexer_lex(lexer, &error);
}
