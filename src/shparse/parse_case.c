#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

s_ast *parse_rule_case(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (!tok_is(tok, TOK_WORD))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected WORD", TOKT_STR(tok));
    return NULL;
  }
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_CASE;
  res->data.ast_case = ACASE(parse_word(lexer, errman), NULL);
  if (ERRMAN_FAILING(errman))
    return res;
  parse_newlines(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_IN))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'in'", TOKT_STR(tok));
    return res;
  }
  parse_newlines(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  res->data.ast_case.nodes = parse_case_clause(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok_free(lexer_pop(lexer, errman), true);
  return res;
}

s_acase_node *parse_case_clause(s_lexer *lexer, s_errman *errman)
{
  s_acase_node *res = parse_case_item(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  s_acase_node *tail = res;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  while (tok_is(tok, TOK_DSEMI))
  {
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    if (tok_is(tok, TOK_ESAC))
      return res;
    s_acase_node *tmp = parse_case_item(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    tail->next = tmp;
    tail = tmp;
  }
  parse_newlines(lexer, errman);
  return res;
}

s_wordlist *parse_pattern(s_lexer *lexer, s_errman *errman)
{
  s_wordlist *res = parse_word(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  s_wordlist *tail = res;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    s_wordlist *tmp = parse_word(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  return res;
}

s_acase_node *parse_case_item(s_lexer *lexer, s_errman *errman)
{
  s_acase_node *res = xcalloc(sizeof(s_acase_node), 1);
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (tok_is(tok, TOK_RPAR))
    tok_free(lexer_pop(lexer, errman), true);
  s_wordlist *pattern = parse_pattern(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  s_ast *action = NULL;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_LPAR))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '('", TOKT_STR(tok));
    return res;
  }
  tok_free(lexer_pop(lexer, errman), true);
  parse_newlines(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  if (!tok_is(tok, TOK_ESAC) && !tok_is(tok, TOK_DSEMI))
    action = parse_compound_list(lexer, errman);
  *res = ACASE_NODE(pattern, action, NULL);
  return res;
}
