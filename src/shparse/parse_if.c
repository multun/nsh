#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"

static bool start_else_clause(const s_token *tok)
{
  return tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_ELIF);
}

s_ast *parse_rule_if(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_IF;
  res->data.ast_if.condition = parse_compound_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_THEN))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'then'", TOKT_STR(tok));
    return res;
  }
  tok_free(lexer_pop(lexer, errman), true);
  res->data.ast_if.success = parse_compound_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (tok_is(tok, TOK_FI))
  {
    tok_free(lexer_pop(lexer, errman), true);
    return res;
  }
  else if (!start_else_clause(tok))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'fi', 'else' or 'elif'",
            TOKT_STR(tok));
    return res;
  }
  res->data.ast_if.failure = parse_else_clause(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_FI))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'fi'", TOKT_STR(tok));
    return  res;
  }
  tok_free(lexer_pop(lexer, errman), true);
  return res;
}

s_ast *parse_else_clause(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  bool elif = tok_is(tok, TOK_ELIF);
  tok_free(lexer_pop(lexer, errman), true);
  if (!elif)
    return parse_compound_list(lexer, errman);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_IF;
  res->data.ast_if.condition = parse_compound_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_THEN))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'then'", TOKT_STR(tok));
    return res;
  }
  tok_free(lexer_pop(lexer, errman), true);
  res->data.ast_if.success = parse_compound_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (!start_else_clause(tok))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'else' or 'elif'", TOKT_STR(tok));
    return res;
  }
  res->data.ast_if.failure = parse_else_clause(lexer, errman);
  return res;
}
