#include "shlex/lexer.h"
#include "utils/alloc.h"

#include <stdlib.h>
#include <string.h>


s_token *tok_alloc(s_lexer *lexer)
{
  s_token *res = xmalloc(sizeof(*res));
  res->lineinfo = lexer->wlexer.cs->line_info;
  res->type = TOK_WORD;
  res->next = NULL;
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


bool tok_is_ass(char *str, bool first)
{
  if (!*str)
    return false;
  if (*str == '=')
    return !first;
  return ((*str >= '0' && *str <= '9' && !first) || *str == '_'
          || (*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z'))
         && tok_is_ass(str + 1, false);
}


bool tok_is(const s_token *tok, enum token_type type)
{
  if (TOK_IS_DET(tok->type) || TOK_IS_DET(type))
    return tok->type == type;

  if (type == TOK_ASSIGNMENT_WORD)
    return tok_is_ass(TOK_STR(tok), true);

  // TODO: handle name
  if (type == TOK_WORD || type == TOK_NAME)
    return true;

  if (TOK_IS_KW(type))
    return !strcmp(g_keywords[TOK_KW_ALIGN(type)], TOK_STR(tok));

  return true;
}
