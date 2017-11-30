#include "shparse/parse.h"
#include "utils/alloc.h"

s_ast *parse_rule_case(s_lexer *lexer)
{
  tok_free(lexer_pop(lexer), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_CASE;
  const s_token *tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_WORD))
    return NULL; // TODO: handle parsing error
  res->data.ast_case = ACASE(parse_word(lexer), NULL);
  parse_newlines(lexer);
  tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_IN))
    return NULL; // TODO: handle parsing error
  parse_newlines(lexer);
  res->data.ast_case.nodes = parse_case_clause(lexer);
  tok_free(lexer_pop(lexer), true);
  // TODO: handle parsing error
  return res;
}

s_acase_node *parse_case_clause(s_lexer *lexer)
{
  s_acase_node *res = parse_case_item(lexer);
  s_acase_node *tail = res;
  const s_token *tok = lexer_peek(lexer);
  while (tok_is(tok, TOK_DSEMI))
  {
    tok_free(lexer_pop(lexer), true);
    parse_newlines(lexer);
    tok = lexer_peek(lexer);
    if (tok_is(tok, TOK_ESAC))
      return res;
    s_acase_node *tmp = parse_case_item(lexer);
    tail->next = tmp;
    tail = tmp;
  }
  parse_newlines(lexer);
  return res;
}

s_wordlist *parse_pattern(s_lexer *lexer)
{
  s_wordlist *res = parse_word(lexer);
  // TODO: handle parsing error
  s_wordlist *tail = res;
  const s_token *tok = lexer_peek(lexer);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer), true);
    s_wordlist *tmp = parse_word(lexer);
    tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer);
  }
  return res;
}

s_acase_node *parse_case_item(s_lexer *lexer)
{
  s_acase_node *res = xmalloc(sizeof(s_acase_node));
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_RPAR))
    tok_free(lexer_pop(lexer), true);
  s_wordlist *pattern = parse_pattern(lexer);
  s_ast *action = NULL;
  tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_LPAR))
    return NULL; // TODO: raise unexpected token
  tok_free(lexer_pop(lexer), true);
  parse_newlines(lexer);
  tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_ESAC) && !tok_is(tok, TOK_DSEMI))
    action = parse_compound_list(lexer);
  *res = ACASE_NODE(pattern, action, NULL);
  return res;
}
