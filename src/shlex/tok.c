#include "shlex/lexer.h"


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
