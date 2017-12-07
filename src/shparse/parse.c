#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_newlines(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return;
  while (tok_is(tok, TOK_NEWLINE))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return;
  }
}


s_ast *parse(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return NULL;
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return NULL;
  s_ast *res = parse_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return res;
  sherror(&tok->lineinfo, errcont,
          "unxpected token %s, expected 'EOF' or '\\n'", TOKT_STR(tok));
  return res;
}


static s_ast *list_loop(s_lexer *lexer, s_errcont *errcont,
                        s_ast *res, const s_token *tok)
{
  s_alist *tmp = &res->data.ast_list;
  while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
      return res;
    s_alist *next = xcalloc(sizeof(s_alist), 1);
    *next = ALIST(parse_and_or(lexer, errcont), NULL);
    tmp->next = next;
    if (ERRMAN_FAILING(errcont))
      return res;
    tmp = next;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
  }
  return res;
}


s_ast *parse_list(s_lexer *lexer, s_errcont *errcont)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_LIST;
  res->data.ast_list = ALIST(parse_and_or(lexer, errcont), NULL);
  if (ERRMAN_FAILING(errcont))
    return res;
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return list_loop(lexer, errcont, res, tok);
}


s_ast *parse_and_or(s_lexer *lexer, s_errcont *errcont)
{
  s_ast *res = parse_pipeline(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;

  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  while (tok_is(tok, TOK_OR_IF) || tok_is(tok, TOK_AND_IF))
  {
    bool or = tok_is(tok, TOK_OR_IF);
    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    s_ast *bool_op = xcalloc(sizeof(s_ast), 1);
    bool_op->type = SHNODE_BOOL_OP;
    bool_op->data.ast_bool_op = ABOOL_OP(or ? BOOL_OR : BOOL_AND,
                                         res, parse_pipeline(lexer, errcont));
    if (ERRMAN_FAILING(errcont))
      return res;
    res = bool_op;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
  }
  return res;
}
