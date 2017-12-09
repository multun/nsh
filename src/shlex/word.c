#include "shlex/breaking.h"
#include "shlex/lexer.h"
#include "utils/error.h"
#include "utils/macros.h"
#include "shlex/lexer_error.h"

#include <ctype.h>


static bool read_single_quote(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  if (cstream_peek(cs) != '\'')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  while ((tok->delim = cstream_pop(cs)) != '\'')
    if (tok->delim == EOF)
      return LEXERROR(&cs->line_info, errcont, "unexpected EOF"
                      " while reading simple quoted string");
    else
      TOK_PUSH(tok, tok->delim);
  TOK_PUSH(tok, tok->delim);
  tok->delim = cstream_peek(cs);
  return true;
}


static bool read_backslash(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  if (tok->delim != '\\')
    return false;

  cstream_pop(cs);
  if ((tok->delim = cstream_pop(cs)) == EOF)
    return !LEXERROR(&cs->line_info, errcont, "can't escape EOF");

  if (tok->delim != '\n')
    TOK_PUSH(tok, tok->delim);

  tok->delim = cstream_peek(cs);
  return true;
}


static bool read_double_quote(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  if (cstream_peek(cs) != '"')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  while ((tok->delim = cstream_peek(cs)) != '"')
  {
    if (tok->delim == EOF)
      return LEXERROR(&cs->line_info, errcont, "unexpected EOF"
                      " while reading double quoted string");
    else if (tok->delim == '\\')
      read_backslash(cs, tok, errcont);
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
typedef bool (*f_tok_reader)(s_cstream *cs, s_token *tok, s_errcont *errcont);

static const f_tok_reader word_readers[] =
{
  read_single_quote,
  read_double_quote,
  read_backslash,
};


static bool skip_spaces(s_cstream *cs, s_token *tok)
{
  while ((tok->delim = cstream_peek(cs)) != EOF)
    if (isblank(tok->delim))
      cstream_pop(cs);
    else
      return false;
  return true;
}


static bool skip_comment(s_cstream *cs, s_token *tok)
{
  if (cstream_peek(cs) != '#')
    return false;

  while ((tok->delim = cstream_peek(cs)) != EOF)
    if (tok->delim == '\n')
      return false;
    else
      cstream_pop(cs);
  return true;
}


static void handle_break(s_cstream *cs, s_token *tok)
{
  if (TOK_SIZE(tok))
    return;
  read_breaking(cs, tok);
}


static bool try_read_word(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  for (size_t i = 0; i < ARR_SIZE(word_readers); i++)
    if (word_readers[i](cs, tok, errcont))
      return false;
  return true;
}



/**
** \brief reads characters from a stream, to a word.
** \param cs the stream to read from
** \param tok the token to write into
** \param error an error location
** \return whether we hit EOF
*/
void word_read(s_cstream *cs, s_token *tok, s_errcont *errcont)
{
  while ((tok->delim = cstream_peek(cs)) != EOF
         && !(!TOK_SIZE(tok) && (skip_spaces(cs, tok)
                                 || skip_comment(cs, tok))))
  {
    if (read_backslash(cs, tok, errcont))
      continue;

    if (is_breaking(tok->delim))
    {
      handle_break(cs, tok);
      break;
    }

    if (try_read_word(cs, tok, errcont))
      TOK_PUSH(tok, cstream_pop(cs));
  }
}
