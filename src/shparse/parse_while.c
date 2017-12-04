#include "shparse/parse.h"
#include "utils/alloc.h"

s_ast *parse_rule_while(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_WHILE;
  res->data.ast_while = AWHILE(parse_compound_list(lexer, errman),
                               parse_do_group(lexer, errman));
  // TODO: handle parsing error
  return res;
}

s_ast *parse_rule_until(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_UNTIL;
  res->data.ast_until = AUNTIL(parse_compound_list(lexer, errman),
                               parse_do_group(lexer, errman));
  // TODO: handle parsing error
  return res;
}

s_ast *parse_do_group(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_DO))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = parse_compound_list(lexer, errman);
  tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_DONE))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer, errman), true);
  return res;
}
