#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <stdlib.h>


s_token *tok_alloc(void)
{
  s_token *res = xmalloc(sizeof(*res));
  res->type = TOK_WORD;
  evect_init(&res->str, TOK_BUF_MIN_SIZE);
  return res;
}


void tok_free(s_token *tok, bool free_buf)
{
  if (free_buf)
    evect_destroy(&tok->str);
  free(tok);
}


bool tok_is(const s_token *tok, enum token_type type)
{
  (void)tok;
  (void)type;
  return false;
}


s_token *tok_as(s_token *tok, enum token_type type)
{
  (void)tok;
  (void)type;
  return NULL;
}
