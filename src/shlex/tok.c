#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <stdlib.h>
#include <string.h>


s_token *tok_alloc(s_lexer *lexer)
{
  s_token *res = xmalloc(sizeof(*res));
  res->specified = false;
  res->lineinfo = lexer->stream->line_info;
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



#define LEX_OPS_MAP(TokName, Value) Value,


static const char *g_keywords[] =
{
  LEX_KW_TOKS(LEX_OPS_MAP)
};


bool tok_is(const s_token *tok, enum token_type type)
{
  if (tok->specified || TOK_IS_DET(type))
    return tok->type == type;

  if (type == TOK_WORD)
    return true;

  if (TOK_IS_KW(type))
    return !strcmp(g_keywords[TOK_KW_ALIGN(type)], TOK_STR(tok));

  // TODO: handle name and assignment word
  return true;
}
