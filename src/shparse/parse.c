#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_newlines(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return;
  while (tok_is(tok, TOK_NEWLINE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return;
  }
}

s_ast *parse(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return NULL;
  s_ast *res = parse_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return res;
  sherror(&tok->lineinfo, errman, "unxpected token %s, expected 'EOF' or '\\n'", TOKT_STR(tok));
  return res;
}

s_ast *parse_list(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_LIST;
  res->data.ast_list = ALIST(parse_and_or(lexer, errman), NULL);
  if (ERRMAN_FAILING(errman))
    return res;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  s_alist *tmp = &res->data.ast_list;
  while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND))
  {
    tok_free(lexer_pop(lexer, errman), true);
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
      return res;
    s_alist *next = xcalloc(sizeof(s_alist), 1);
    *next = ALIST(parse_and_or(lexer, errman), NULL);
    if (ERRMAN_FAILING(errman))
      return res;
    tmp->next = next;
    tmp = next;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  return res;
}

s_ast *parse_and_or(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = parse_pipeline(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  while (tok_is(tok, TOK_OR_IF) || tok_is(tok, TOK_AND_IF))
  {
    bool or = tok_is(tok, TOK_OR_IF);
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    s_ast *bool_op = xcalloc(sizeof(s_ast), 1);
    bool_op->type = SHNODE_BOOL_OP;
    bool_op->data.ast_bool_op = ABOOL_OP(or ? BOOL_OR : BOOL_AND,
                                         res, parse_pipeline(lexer, errman));
    if (ERRMAN_FAILING(errman))
      return res;
    res = bool_op;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  return res;
}
