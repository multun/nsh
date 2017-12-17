#include "shlex/lexer.h"
#include "utils/alloc.h"
#include "shexec/clean_exit.h"

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
  while (lexer->head)
  {
    s_token *tok = lexer->head;
    lexer->head = tok->next;
    tok_free(tok, true);
  }
  free(lexer);
}


static bool is_only_digits(s_token *tok)
{
  for (size_t i = 0; i < TOK_SIZE(tok); i++)
    if (!isdigit(TOK_STR(tok)[i]))
      return false;
  return true;
}


static void lexer_lex(s_token **tres, s_lexer *lexer, s_errcont *errcont)
{
  s_token *res = *tres = tok_alloc(lexer);
  word_read(lexer->stream, res, errcont);

  if (!TOK_SIZE(res) && res->delim == EOF)
  {
    res->type = TOK_EOF;
    res->specified = true;
    return;
  }

  for (size_t i = 0; i < TOK_SIZE(res); i++)
    clean_assert(errcont, res->str.data[i], "no input NUL bytes are allowed");

  // this case is super annoying to handle inside word_read
  // TODO: move inside word_read and deepen the abstraction
  if (is_only_digits(res)
      && (res->delim == '<' || res->delim == '>'))
  {
    res->type = TOK_IO_NUMBER;
    res->specified = true;
  }

  TOK_PUSH(res, '\0');
  return;
}


s_token *lexer_peek_at(s_lexer *lexer, s_token *tok, s_errcont *errcont)
{
  if (!tok->next)
    lexer_lex(&tok->next, lexer, errcont);
  return tok->next;
}


s_token *lexer_peek(s_lexer *lexer, s_errcont *errcont)
{
  if (!lexer->head)
    lexer_lex(&lexer->head, lexer, errcont);
  return lexer->head;
}


s_token *lexer_pop(s_lexer *lexer, s_errcont *errcont)
{
  if (!lexer->head)
    lexer_lex(&lexer->head, lexer, errcont);

  s_token *ret = lexer->head;
  lexer->head = ret->next;
  return ret;
}
