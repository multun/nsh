#include "shparse/parse.h"
#include "utils/alloc.h"

s_ast *parse_rule_case(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_CASE;
  const s_token *tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_WORD))
    return NULL; // TODO: handle parsing error
  res->data.ast_case = ACASE(parse_word(lexer, errman), NULL);
  parse_newlines(lexer, errman);
  tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_IN))
    return NULL; // TODO: handle parsing error
  parse_newlines(lexer, errman);
  res->data.ast_case.nodes = parse_case_clause(lexer, errman);
  tok_free(lexer_pop(lexer, errman), true);
  // TODO: handle parsing error
  return res;
}

s_acase_node *parse_case_clause(s_lexer *lexer, s_errman *errman)
{
  s_acase_node *res = parse_case_item(lexer, errman);
  s_acase_node *tail = res;
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_DSEMI))
  {
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    tok = lexer_peek(lexer, errman);
    if (tok_is(tok, TOK_ESAC))
      return res;
    s_acase_node *tmp = parse_case_item(lexer, errman);
    tail->next = tmp;
    tail = tmp;
  }
  parse_newlines(lexer, errman);
  return res;
}

s_wordlist *parse_pattern(s_lexer *lexer, s_errman *errman)
{
  s_wordlist *res = parse_word(lexer, errman);
  // TODO: handle parsing error
  s_wordlist *tail = res;
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    s_wordlist *tmp = parse_word(lexer, errman);
    tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer, errman);
  }
  return res;
}

s_acase_node *parse_case_item(s_lexer *lexer, s_errman *errman)
{
  s_acase_node *res = xmalloc(sizeof(s_acase_node));
  const s_token *tok = lexer_peek(lexer, errman);
  if (tok_is(tok, TOK_RPAR))
    tok_free(lexer_pop(lexer, errman), true);
  s_wordlist *pattern = parse_pattern(lexer, errman);
  s_ast *action = NULL;
  tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_LPAR))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer, errman), true);
  parse_newlines(lexer, errman);
  tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_ESAC) && !tok_is(tok, TOK_DSEMI))
    action = parse_compound_list(lexer, errman);
  *res = ACASE_NODE(pattern, action, NULL);
  return res;
}
