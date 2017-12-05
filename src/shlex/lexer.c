#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <assert.h>
#include <err.h>
#include <ctype.h>
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


static bool is_only_digits(s_token *tok)
{
  for (size_t i = 0; i < TOK_SIZE(tok); i++)
    if (!isdigit(TOK_STR(tok)[i]))
      return false;
  return true;
}


static s_token *lexer_lex(s_lexer *lexer, s_errman *errman)
{
  assert(!ERRMAN_FAILING(errman));
  s_token *res = tok_alloc(lexer);
  word_read(lexer->stream, res, errman);

  if (ERRMAN_FAILING(errman))
  {
    tok_free(res, true);
    return NULL;
  }

  if (!TOK_SIZE(res) && res->delim == EOF)
  {
    res->type = TOK_EOF;
    res->specified = true;
    return res;
  }


  // this case is super annoying to handle inside word_read
  // TODO: move inside word_read and deepen the abstraction
  if (is_only_digits(res)
      && (res->delim == '<' || res->delim == '>'))
    res->type = TOK_IO_NUMBER;

  TOK_PUSH(res, '\0');
  return res;
}


void lexer_push(s_lexer *lexer, s_token *tok)
{
  tok->next = lexer->head;
  lexer->head = tok;
}


const s_token *lexer_peek(s_lexer *lexer, s_errman *errman)
{
  if (!lexer->head)
  {
    s_token *ntok = lexer_lex(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return NULL;
    lexer->head = ntok;
  }
  return lexer->head;
}


s_token *lexer_pop(s_lexer *lexer, s_errman *errman)
{
  if (lexer->head)
  {
    s_token *ret = lexer->head;
    lexer->head = ret->next;
    return ret;
  }

  return lexer_lex(lexer, errman);
}
