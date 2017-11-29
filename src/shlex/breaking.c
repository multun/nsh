#include "shlex/breaking.h"
#include "utils/error.h"

#include <stddef.h>
#include <ctype.h>


static bool is_operator(char c, size_t pos)
{
  (void)pos;
  // TODO: actual selection
  return c == '<';
}


bool is_breaking(char c, size_t pos)
{
  return isblank(c) || c == '\n' || is_operator(c, pos);
}


static bool read_operator(s_cstream *cs, s_token *tok, s_sherror **error)
{
  (void)error;
  // TODO: actual code
  TOK_PUSH(tok, cstream_pop(cs));
  return false;
}


bool read_breaking(s_cstream *cs, s_token *tok, s_sherror **error)
{
  // TODO: handle \n separatly
  return read_operator(cs, tok, error);
}
