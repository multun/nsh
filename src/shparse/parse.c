#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

void parse_newlines(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_NEWLINE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    tok = lexer_peek(lexer, errman);
  }
}

s_ast *parse(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return NULL;
  s_ast *res = parse_list(lexer, errman);
  // TODO: handle parsing error
  tok = lexer_peek(lexer, errman);
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return res;
  // TODO: parsing failed unxepected token exected EOF or '\n'
  return NULL;
}

s_ast *parse_list(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_LIST;
  res->data.ast_list = ALIST(parse_and_or(lexer, errman), NULL);
  const s_token *tok = lexer_peek(lexer, errman);
  s_alist *tmp = &res->data.ast_list;
  while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND))
  {
    tok_free(lexer_pop(lexer, errman), true);
    tok = lexer_peek(lexer, errman);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
      return res;
    s_alist *next = xmalloc(sizeof(s_alist));
    *next = ALIST(parse_and_or(lexer, errman), NULL);
    // TODO: handle parsing error
    tmp->next = next;
    tmp = next;
    tok = lexer_peek(lexer, errman);
  }
  return res;
}

s_ast *parse_and_or(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = parse_pipeline(lexer, errman);
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_OR_IF) || tok_is(tok, TOK_AND_IF))
  {
    bool or = tok_is(tok, TOK_OR_IF);
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    s_ast *bool_op = xmalloc(sizeof(s_ast));
    bool_op->type = SHNODE_BOOL_OP;
    bool_op->data.ast_bool_op = ABOOL_OP(or ? BOOL_OR : BOOL_AND,
                                         res, parse_pipeline(lexer, errman));
    // TODO: handle parsing exception
    res = bool_op;
    tok = lexer_peek(lexer, errman);
  }
  return res;
}
