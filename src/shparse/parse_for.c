#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static s_wordlist *for_word_loop(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  s_wordlist *res = NULL;
  s_wordlist *tail = NULL;
  while (tok_is(tok, TOK_WORD))
  {
    s_token *w = lexer_pop(lexer);
    s_wordlist *tmp = xmalloc(sizeof(s_wordlist));
    *tmp = WORDLIST(TOK_STR(w), true, true, NULL);
    if (!res)
      res = tmp;
    else
      tail->next = tmp;
    tail = tmp;
    //tok_free(w);
    tok = lexer_peek(lexer);
  }
  // TODO: handle parsing error
  return res;
}

static s_wordlist *parse_in(s_lexer *lexer, bool *in)
{
  s_wordlist *words = NULL;
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_NEWLINE))
  {
    while (tok_is(tok, TOK_NEWLINE))
    {
      /*tok_free*/(lexer_pop(lexer));
      tok = lexer_peek(lexer);
    }
    if (tok_is(tok, TOK_IN))
    {
      *in = true;
      /*tok_free*/(lexer_pop(lexer));
      words = for_word_loop(lexer);
    }
  }
  return words;
}

s_ast *parse_rule_for(s_lexer *lexer)
{
  /*tok_free*/(lexer_pop(lexer));
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_FOR;
  s_wordlist *name = parse_word(lexer); // TODO: handle parsing error
  bool in = false;
  s_wordlist *words = parse_in(lexer, &in);
  const s_token *tok = lexer_peek(lexer);
  if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI))
    return NULL; // TODO: handle parsing error
  /*tok_free*/(lexer_pop(lexer));
  tok = lexer_peek(lexer);
  while (tok_is(tok, TOK_NEWLINE))
  {
    /*tok_free*/(lexer_pop(lexer));
    tok = lexer_peek(lexer);
  }
  res->data.ast_for = AFOR(name, words, parse_do_group(lexer));
  return res; // TODO: handle parsing error
}
