#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"


s_wordlist *parse_word(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (!tok_is(tok, TOK_WORD))
  {
    sherror(&tok->lineinfo, errman, "unexpected token %s, expected WORD", TOKT_STR(tok));
    return NULL;
  }

  s_wordlist *res = xmalloc(sizeof(s_wordlist));
  s_token *wrd = lexer_pop(lexer, errman);
  *res = WORDLIST(TOK_STR(wrd), true, true, NULL);
  tok_free(wrd, false);
  return res;
}
