#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"


s_ast *parse_rule_while(s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_WHILE;
  res->data.ast_while = AWHILE(parse_compound_list(lexer, errcont), NULL);
  if (ERRMAN_FAILING(errcont))
    return res;
  res->data.ast_while.actions = parse_do_group(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return res;
}


s_ast *parse_rule_until(s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_UNTIL;
  res->data.ast_until = AUNTIL(parse_compound_list(lexer, errcont),NULL);
  if (ERRMAN_FAILING(errcont))
    return res;
  res->data.ast_until.actions = parse_do_group(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return res;
}


s_ast *parse_do_group(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return NULL;
  if (!tok_is(tok, TOK_DO))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'do'", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errcont), true);
  s_ast *res = parse_compound_list(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_DONE))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'done'", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errcont), true);
  return res;
}
