#include "shlex/breaking.h"
#include "shlex/lexer.h"
#include "utils/error.h"
#include "utils/macros.h"

#include <ctype.h>


static bool read_single_quote(s_cstream *cs, s_token *tok, s_errman *errman)
{
  if (cstream_peek(cs) != '\'')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  while ((tok->delim = cstream_pop(cs)) != '\'')
    if (tok->delim == EOF)
      return sherror(&cs->line_info, errman, "unexpected EOF"
                     " while reading simple quoted string");
    else
      TOK_PUSH(tok, tok->delim);
  TOK_PUSH(tok, tok->delim);
  tok->delim = cstream_peek(cs);
  return true;
}


static bool read_backslash(s_cstream *cs, s_token *tok, s_errman *errman)
{
  if (tok->delim != '\\')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  if ((tok->delim = cstream_pop(cs)) == EOF)
    return sherror(&cs->line_info, errman, "can't escape EOF");

  if (tok->delim != '\n')
    TOK_PUSH(tok, tok->delim);

  tok->delim = cstream_peek(cs);
  return true;
}


static bool read_double_quote(s_cstream *cs, s_token *tok, s_errman *errman)
{
  if (cstream_peek(cs) != '"')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  while ((tok->delim = cstream_peek(cs)) != '"')
  {
    if (tok->delim == EOF)
      return sherror(&cs->line_info, errman, "unexpected EOF"
                     " while reading double quoted string");
    else if (tok->delim == '\\')
    {
      read_backslash(cs, tok, errman);
      if (ERRMAN_FAILING(errman))
        return true;
    }
    else
      TOK_PUSH(tok, cstream_pop(cs));
  }
  TOK_PUSH(tok, cstream_pop(cs));
  tok->delim = cstream_peek(cs);
  return true;
}


/**
** \return whether some special sequence was read
*/
typedef bool (*f_tok_reader)(s_cstream *cs, s_token *tok, s_errman *errman);

static const f_tok_reader word_readers[] =
{
  read_single_quote,
  read_double_quote,
  read_backslash,
};


static void skip_spaces(s_cstream *cs)
{
  int c;
  while ((c = cstream_peek(cs)) != EOF)
    if (isblank(c))
      cstream_pop(cs);
    else
      break;
}


static void handle_break(s_cstream *cs, s_token *tok)
{
  if (TOK_SIZE(tok))
    return;
  read_breaking(cs, tok);
}


/**
** \brief reads characters from a stream, to a word.
** \param cs the stream to read from
** \param tok the token to write into
** \param error an error location
** \return whether we hit EOF
*/
void word_read(s_cstream *cs, s_token *tok, s_errman *errman)
{
  skip_spaces(cs);
  while ((tok->delim = cstream_peek(cs)) != EOF)
  {
    if (is_breaking(tok->delim))
    {
      handle_break(cs, tok);
      break;
    }

    bool push = true;
    for (size_t i = 0; i < ARR_SIZE(word_readers); i++)
      if (word_readers[i](cs, tok, errman))
      {
        push = false;
        if (ERRMAN_FAILING(errman))
          return;
      }

    if (push)
      TOK_PUSH(tok, cstream_pop(cs));
  }
  return;
}
