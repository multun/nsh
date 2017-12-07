#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"


static s_wordlist *for_word_loop(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  s_wordlist *res = NULL;
  if (ERRMAN_FAILING(errcont))
    return res;
  s_wordlist *tail = NULL;
  while (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
  {
    s_wordlist *tmp = parse_word(lexer, errcont);
    if (!res)
      res = tmp;
    else
      tail->next = tmp;
    if (ERRMAN_FAILING(errcont))
      return res;
    tail = tmp;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return res;
  }
  tok_free(lexer_pop(lexer, errcont), true);
  return res;
}


static s_wordlist *parse_in(s_lexer *lexer, s_errcont *errcont)
{
  s_wordlist *words = NULL;
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return words;
  if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN))
  {
    parse_newlines(lexer, errcont);
    if (ERRMAN_FAILING(errcont) || ((tok = lexer_peek(lexer, errcont))
                                    && (ERRMAN_FAILING(errcont))))
      return words;
    if (tok_is(tok, TOK_IN))
    {
      tok_free(lexer_pop(lexer, errcont), true);
      words = for_word_loop(lexer, errcont);
      if (ERRMAN_FAILING(errcont))
        return words;
      tok = lexer_peek(lexer, errcont);
      if (ERRMAN_FAILING(errcont))
        return words;
    }
  }
  if (tok_is(tok, TOK_SEMI))
    tok_free(lexer_pop(lexer, errcont), true);
  return words;
}


static bool parse_collection(s_lexer *lexer, s_errcont *errcont,
                             s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return false;
  if (!tok_is(tok, TOK_DO))
  {
    if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI)
        && !tok_is(tok, TOK_IN))
    {
      sherror(&tok->lineinfo, errcont,
              "unexpected token %s, expected 'do', ';' or '\\n'",
              TOKT_STR(tok));
      return false;
    }
    res->data.ast_for.collection = parse_in(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return false;
    parse_newlines(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return false;
    tok = lexer_peek(lexer, errcont);
    if (ERRMAN_FAILING(errcont))
      return false;
  }
  return true;
}


s_ast *parse_rule_for(s_lexer *lexer, s_errcont *errcont)
{
  tok_free(lexer_pop(lexer, errcont), true);
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_FOR;
  s_wordlist *name = parse_word(lexer, errcont);
  res->data.ast_for.var = name;
  if (ERRMAN_FAILING(errcont))
    return res;

  if (!parse_collection(lexer, errcont, res))
    return res;

  const s_token *tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_DO))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected 'do'", TOKT_STR(tok));
    return res;
  }
  res->data.ast_for.actions = parse_do_group(lexer, errcont);
  return res;
}
