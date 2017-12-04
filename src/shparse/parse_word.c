#include "utils/alloc.h"
#include "shparse/parse.h"


s_wordlist *parse_word(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (!tok_is(tok, TOK_WORD))
    return NULL; // TODO: handle error

  s_wordlist *res = xmalloc(sizeof(s_wordlist));
  s_token *wrd = lexer_pop(lexer, errman);
  *res = WORDLIST(TOK_STR(wrd), true, true, NULL);
  tok_free(wrd, false);
  return res;
}
