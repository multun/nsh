#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <stdlib.h>
#include <string.h>


void tok_alloc(s_lexer *lexer)
{
  s_token *res = xmalloc(sizeof(*res));
  res->specified = false;
  res->lineinfo = lexer->stream->line_info;
  res->type = TOK_WORD;
  res->next = lexer->head;
  evect_init(&res->str, TOK_BUF_MIN_SIZE);
  lexer->head = res;
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

  if (type == TOK_ASSIGNMENT_WORD)
    return TOK_STR(tok)[0] != '=' && !!strchr(TOK_STR(tok), '=');

  // TODO: handle name
  if (type == TOK_WORD || type == TOK_NAME)
    return true;

  if (TOK_IS_KW(type))
    return !strcmp(g_keywords[TOK_KW_ALIGN(type)], TOK_STR(tok));

  return true;
}
