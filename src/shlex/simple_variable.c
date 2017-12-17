#include "shlex/lexer.h"
#include "shlex/lexer_error.h"


static bool read_digit(int c)
{
  return c >= '0' && c <= '9';
}


static bool read_name_char(int c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'
          || read_digit(c);
}


bool read_braket(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  if (cstream_peek(cs) != '{' || !tok->str.size
      || tok->str.data[tok->str.size - 1] != '$')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  bool num = read_digit(cstream_peek(cs));
  bool first = true;
  bool err = false;
  while ((tok->delim = cstream_peek(cs)) != EOF)
  {
    if (tok->delim == '}')
    {
      TOK_PUSH(tok, cstream_pop(cs));
      if (first || err)
        break;
      return true;
    }
    first = false;
    if ((read_name_char(tok->delim) && num) || !read_name_char(tok->delim))
      err = true;
    TOK_PUSH(tok, cstream_pop(cs));
  }
  if (tok->delim != EOF)
    LEXERROR(&cs->line_info, errcont, "%s: bad substitution", tok->str.data);
  LEXERROR(&cs->line_info, errcont, "unexpected EOF"
           " while reading variable name");
}
