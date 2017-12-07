#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"

static bool start_else_clause(const s_token *tok)
{
  return tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_ELIF);
}


static void parse_rule_if_end(s_lexer *lexer, s_errcont *errcont, s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (tok_is(tok, TOK_FI))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    return;
  }
  else if (!start_else_clause(tok))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'fi', 'else' or 'elif'",
            TOKT_STR(tok));
  parse_else_clause(&res->data.ast_if.failure, lexer, errcont);

  tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_FI))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'fi'", TOKT_STR(tok));
  tok_free(lexer_pop(lexer, errcont), true);
}


void parse_rule_if(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_IF;
  parse_compound_list(&(*res)->data.ast_if.condition, lexer, errcont);

  const s_token *tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_THEN))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'then'", TOKT_STR(tok));

  tok_free(lexer_pop(lexer, errcont), true);
  parse_compound_list(&(*res)->data.ast_if.success, lexer, errcont);
  parse_rule_if_end(lexer, errcont, *res);
}


static void parse_else_clause_end(s_lexer *lexer, s_errcont *errcont,
                                  s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_THEN))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'then'", TOKT_STR(tok));
  tok_free(lexer_pop(lexer, errcont), true);
  parse_compound_list(&res->data.ast_if.success, lexer, errcont);
  tok = lexer_peek(lexer, errcont);
  if (!start_else_clause(tok))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'else' or 'elif'", TOKT_STR(tok));
  parse_else_clause(&res->data.ast_if.failure, lexer, errcont);
}


void parse_else_clause(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  bool elif = tok_is(tok, TOK_ELIF);
  tok_free(lexer_pop(lexer, errcont), true);
  if (!elif)
  {
    parse_compound_list(res, lexer, errcont);
    return;
  }
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_IF;
  parse_compound_list(&(*res)->data.ast_if.condition, lexer, errcont);
  parse_else_clause_end(lexer, errcont, *res);
}
