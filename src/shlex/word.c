#include "shlex/breaking.h"
#include "shlex/lexer.h"
#include "utils/error.h"
#include "utils/macros.h"

#include <ctype.h>


static bool read_single_quote(s_cstream *cs, s_token *tok, s_sherror **error)
{
  (void)error;
  if (cstream_peek(cs) != '\'')
    return false;

  TOK_PUSH(tok, cstream_pop(cs));
  int c;
  while ((c = cstream_pop(cs)) != '\'')
    if (c == -1)
      // TODO: error message
      return true;
    else
      TOK_PUSH(tok, c);
  TOK_PUSH(tok, c);

  return true;
}


static bool read_backslash(s_cstream *cs, s_token *tok, s_sherror **error)
{
  (void)error;
  if (cstream_peek(cs) != '\\')
    return false;

  int c = cstream_pop(cs);
  // TODO: handle errors
  if (c != '\n')
    TOK_PUSH(tok, c);
  return true;
}

static bool read_double_quote(s_cstream *cs, s_token *tok, s_sherror **error)
{
  (void)cs;
  (void)tok;
  (void)error;
  return false;
}


/**
** \return whether some special sequence was read
*/
typedef bool (*f_tok_reader)(s_cstream *cs, s_token *tok, s_sherror **error);

static const f_tok_reader word_readers[] =
{
  read_single_quote,
  read_double_quote,
  read_backslash,
};


static void skip_spaces(s_cstream *cs)
{
  int c;
  while ((c = cstream_peek(cs)) != -1)
    if (isblank(c))
      cstream_pop(cs);
    else
      break;
}


static bool handle_break(s_cstream *cs, s_token *tok)
{
  if (TOK_SIZE(tok))
    return false;
  read_breaking(cs, tok);
  return true;
}


/**
** \brief reads characters from a stream, to a word.
** \param cs the stream to read from
** \param tok the token to write into
** \param quoted whether we're inside a double quote
*/
bool word_read(s_cstream *cs, s_token *tok, s_sherror **error)
{
  skip_spaces(cs);
  do {
    int c = cstream_peek(cs);
    if (c == -1)
      return false;

    if (is_breaking(c))
      return handle_break(cs, tok);

    bool push = true;
    for (size_t i = 0; i < ARR_SIZE(word_readers); i++)
      if (word_readers[i](cs, tok, error))
      {
        push = false;
        if (*error)
          return true;
      }

    if (push)
      TOK_PUSH(tok, cstream_pop(cs));
  } while (true);
  return false;
}
