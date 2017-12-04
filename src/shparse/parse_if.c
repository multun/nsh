#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static bool start_else_clause(const s_token *tok)
{
  return tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_ELIF);
}

s_ast *parse_rule_if(s_lexer *lexer)
{
  tok_free(lexer_pop(lexer), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_IF;
  s_ast *cond = parse_compound_list(lexer);
  // TODO: handle parsing error
  const s_token *tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_THEN))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer), true);
  s_ast *then = parse_compound_list(lexer);
  // TODO: handle parsing error
  res->data.ast_if = AIF(cond, then, NULL);
  tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_FI))
  {
    tok_free(lexer_pop(lexer), true);
    return res;
  }
  else if (!start_else_clause(tok))
    return NULL; // TODO: raise unexpected token
  res->data.ast_if.failure = parse_else_clause(lexer);
  tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_FI))
    return  NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer), true);
  return res;
}

s_ast *parse_else_clause(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  bool elif = tok_is(tok, TOK_ELIF);
  tok_free(lexer_pop(lexer), true);
  if (!elif)
    return parse_compound_list(lexer);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_IF;
  s_ast *cond = parse_compound_list(lexer);
  tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_THEN))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer), true);
  s_ast *then = parse_compound_list(lexer);
  // TODO: handle parsing error
  res->data.ast_if = AIF(cond, then, NULL);
  tok = lexer_peek(lexer);
  if (!start_else_clause(tok))
    return res; // TODO: raise unexpected token
  res->data.ast_if.failure = parse_else_clause(lexer);
  return res;
}
