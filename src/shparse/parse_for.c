#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static s_wordlist *for_word_loop(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  s_wordlist *res = NULL;
  s_wordlist *tail = NULL;
  while (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
  {
    s_wordlist *tmp = parse_word(lexer);
    if (!res)
      res = tmp;
    else
      tail->next = tmp;
    tail = tmp;
    tok = lexer_peek(lexer);
  }
  // TODO: handle parsing error
  tok_free(lexer_pop(lexer), true);
  return res;
}

static s_wordlist *parse_in(s_lexer *lexer, bool *in)
{
  s_wordlist *words = NULL;
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN))
  {
    parse_newlines(lexer);
    tok = lexer_peek(lexer);
    if (tok_is(tok, TOK_IN))
    {
      *in = true;
      tok_free(lexer_pop(lexer), true);
      words = for_word_loop(lexer);
    }
  }
  return words;
}

s_ast *parse_rule_for(s_lexer *lexer)
{
  tok_free(lexer_pop(lexer), true);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_FOR;
  s_wordlist *name = parse_word(lexer); // TODO: handle parsing error
  s_wordlist *words = NULL;
  bool in = false;
  const s_token *tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_DO))
  {
    if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI))
      return NULL; // TODO: handle parsing error
    words = parse_in(lexer, &in);
    parse_newlines(lexer);
    tok = lexer_peek(lexer);
  }
  if (!tok_is(tok, TOK_DO))
    return NULL; // TODO: raise unexpected token
  res->data.ast_for = AFOR(name, words, parse_do_group(lexer));
  return res; // TODO: handle parsing error
}
