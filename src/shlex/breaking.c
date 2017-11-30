#include "shlex/breaking.h"
#include "utils/error.h"
#include "utils/macros.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>


#define LEX_OPS_MAP(TokName, Value) { Value, sizeof(Value) - 1, TokName },


static const struct operator
{
  const char *repr;
  size_t repr_size;
  enum token_type type;
} g_operators[] =
{
  LEX_OP_TOKS(LEX_OPS_MAP)
};


static bool starts_operator(char c)
{
  for (size_t i = 0; i < ARR_SIZE(g_operators); i++)
    if (c ==  g_operators[i].repr[0])
      return true;
  return false;
}


bool is_breaking(char c)
{
  return isblank(c) || c == '\n' || starts_operator(c);
}


static const struct operator *recognise_operator(s_token *tok, char c)
{
  for (size_t i = 0; i < ARR_SIZE(g_operators); i++)
    // TODO: handle null bytes
    if (!strncmp(TOK_STR(tok), g_operators[i].repr, TOK_SIZE(tok))
        && g_operators[i].repr[TOK_SIZE(tok)] == c)
      return &g_operators[i];
  return NULL;
}


static void read_operator(s_cstream *cs, s_token *tok)
{
  const struct operator *op;

  while (true)
  {
    // even if peek fails, we should have read at least a single op char
    if ((tok->delim = cstream_peek(cs)) == -1)
    {
      assert(TOK_SIZE(tok));
      break;
    }
    const struct operator *next_op = recognise_operator(tok, tok->delim);
    if (!next_op)
      break;
    op = next_op;
    TOK_PUSH(tok, cstream_pop(cs));
  }
  assert(op);
  tok->type = op->type;
}


void read_breaking(s_cstream *cs, s_token *tok)
{
  if (cstream_peek(cs) == '\n')
  {
    tok->type = TOK_NEWLINE;
    TOK_PUSH(tok, cstream_pop(cs));
    tok->delim = cstream_peek(cs);
  }
  else
    read_operator(cs, tok);
  tok->specified = true;
}
