#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"

static bool start_else_clause(const s_token *tok)
{
  return tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_ELIF);
}


static s_ast *parse_rule_if_end(s_lexer *lexer, s_errcont *errcont, s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (tok_is(tok, TOK_FI))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    return res;
  }
  else if (!start_else_clause(tok) && sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'fi', 'else' or 'elif'",
            TOKT_STR(tok)))
    return res;
  res->data.ast_if.failure = parse_else_clause(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_FI) && sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'fi'", TOKT_STR(tok)))
    return  res;
  tok_free(lexer_pop(lexer, errcont), true);
  return res;
}


s_ast *parse_rule_if(s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_IF;
  res->data.ast_if.condition = parse_compound_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_THEN) && sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'then'", TOKT_STR(tok)))
    return res;
  tok_free(lexer_pop(lexer, errcont), true);
  res->data.ast_if.success = parse_compound_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return parse_rule_if_end(lexer, errcont, res);
}


static s_ast *parse_else_clause_end(s_lexer *lexer, s_errcont *errcont,
                                    s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_THEN) && sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'then'", TOKT_STR(tok)))
    return res;
  tok_free(lexer_pop(lexer, errcont), true);
  res->data.ast_if.success = parse_compound_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  tok = lexer_peek(lexer, errcont);
  if (!start_else_clause(tok) && sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'else' or 'elif'", TOKT_STR(tok)))
    return res;
  res->data.ast_if.failure = parse_else_clause(lexer, errcont);
  return res;
}


s_ast *parse_else_clause(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return NULL;
  bool elif = tok_is(tok, TOK_ELIF);
  tok_free(lexer_pop(lexer, errcont), true);
  if (!elif)
    return parse_compound_list(lexer, errcont);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_IF;
  res->data.ast_if.condition = parse_compound_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return parse_else_clause_end(lexer, errcont, res);
}
