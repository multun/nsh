#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"


static s_ast *rule_case(s_lexer *lexer, s_errcont *errcont,
                        s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_IN))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'in'", TOKT_STR(tok));
    return res;
  }
  tok_free(lexer_pop(lexer, errcont), true);
  parse_newlines(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  res->data.ast_case.nodes = parse_case_clause(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  tok_free(lexer_pop(lexer, errcont), true);
  return res;
}


s_ast *parse_rule_case(s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return NULL;
  if (!tok_is(tok, TOK_WORD))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected WORD", TOKT_STR(tok));
    return NULL;
  }
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_CASE;
  res->data.ast_case = ACASE(parse_word(lexer, errcont), NULL);

  if (ERRMAN_FAILING(errcont))
    return res;
  parse_newlines(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  return rule_case(lexer, errcont, res);
}


static s_acase_node *case_clause_loop(s_lexer *lexer, s_errcont *errcont,
                                 s_acase_node *res, s_acase_node *tail)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  while (tok_is(tok, TOK_DSEMI))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    if (tok_is(tok, TOK_ESAC))
      return res;
    s_acase_node *tmp = parse_case_item(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
  }
  return res;
}


s_acase_node *parse_case_clause(s_lexer *lexer, s_errcont *errcont)
{
  s_acase_node *res = parse_case_item(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  s_acase_node *tail = res;
  parse_newlines(lexer, errcont);
  return case_clause_loop(lexer, errcont, res, tail);
}


s_wordlist *parse_pattern(s_lexer *lexer, s_errcont *errcont)
{
  s_wordlist *res = parse_word(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  s_wordlist *tail = res;
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    s_wordlist *tmp = parse_word(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
    tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
  }
  return res;
}


static s_acase_node *case_item_loop(s_lexer *lexer, s_errcont *errcont,
                                    s_acase_node *res)
{
  parse_newlines(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_ESAC) && !tok_is(tok, TOK_DSEMI))
    res->action = parse_compound_list(lexer, errcont);
  return res;
}


s_acase_node *parse_case_item(s_lexer *lexer, s_errcont *errcont)
{
  s_acase_node *res = xcalloc(sizeof(s_acase_node), 1);
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (tok_is(tok, TOK_LPAR))
    tok_free(lexer_pop(lexer, errcont), true);
  res->pattern = parse_pattern(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;

  tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return res;
  if (!tok_is(tok, TOK_RPAR))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected ')'", TOKT_STR(tok));
    return res;
  }
  tok_free(lexer_pop(lexer, errcont), true);
  return case_item_loop(lexer, errcont, res);
}
