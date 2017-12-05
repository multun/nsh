#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"


static s_wordlist *for_word_loop(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  s_wordlist *res = NULL;
  if (ERRMAN_FAILING(errman))
    return res;
  s_wordlist *tail = NULL;
  while (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
  {
    s_wordlist *tmp = parse_word(lexer, errman);
    if (!res)
      res = tmp;
    else
      tail->next = tmp;
    if (ERRMAN_FAILING(errman))
      return res;
    tail = tmp;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  tok_free(lexer_pop(lexer, errman), true);
  return res;
}


static s_wordlist *parse_in(s_lexer *lexer, s_errman *errman)
{
  s_wordlist *words = NULL;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return words;
  if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN))
  {
    parse_newlines(lexer, errman);
    if (ERRMAN_FAILING(errman) || ((tok = lexer_peek(lexer, errman))
                                    && (ERRMAN_FAILING(errman))))
      return words;
    if (tok_is(tok, TOK_IN))
    {
      tok_free(lexer_pop(lexer, errman), true);
      words = for_word_loop(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return words;
      tok = lexer_peek(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return words;
    }
  }
  if (tok_is(tok, TOK_SEMI))
    tok_free(lexer_pop(lexer, errman), true);
  return words;
}


static bool parse_collection(s_lexer *lexer, s_errman *errman,
                             s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  if (!tok_is(tok, TOK_DO))
  {
    if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI)
        && !tok_is(tok, TOK_IN))
    {
      sherror(&tok->lineinfo, errman,
              "unexpected token %s, expected 'do', ';' or '\\n'",
              TOKT_STR(tok));
      return false;
    }
    res->data.ast_for.collection = parse_in(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
    parse_newlines(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
  }
  return true;
}


s_ast *parse_rule_for(s_lexer *lexer, s_errman *errman)
{
  tok_free(lexer_pop(lexer, errman), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_FOR;
  s_wordlist *name = parse_word(lexer, errman);
  res->data.ast_for.var = name;
  if (ERRMAN_FAILING(errman))
    return res;

  if (!parse_collection(lexer, errman, res))
    return res;

  const s_token *tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_DO))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected 'do'", TOKT_STR(tok));
    return res;
  }
  res->data.ast_for.actions = parse_do_group(lexer, errman);
  return res;
}
