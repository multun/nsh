#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

s_ast *parse_rule_while(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_WHILE;
  res->data.ast_while = AWHILE(parse_compound_list(lexer, errman), NULL);
  if (ERRMAN_FAILING(errman))
    return res;
  res->data.ast_while.actions = parse_do_group(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  return res;
}

s_ast *parse_rule_until(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_UNTIL;
  res->data.ast_until = AUNTIL(parse_compound_list(lexer, errman),NULL);
  if (ERRMAN_FAILING(errman))
    return res;
  res->data.ast_until.actions = parse_do_group(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  return res;
}

s_ast *parse_do_group(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (!tok_is(tok, TOK_DO))
  {
    sherror(&tok->lineinfo, errman, "unexpected token %s, expected 'do'", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = parse_compound_list(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_DONE))
  {
    sherror(&tok->lineinfo, errman, "unexpected token %s, expected 'done'", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errman), true);
  return res;
}
