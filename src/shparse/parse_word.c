#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"


s_wordlist *parse_word(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (ERRMAN_FAILING(errcont))
    return NULL;
  if (!tok_is(tok, TOK_WORD))
  {
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected WORD", TOKT_STR(tok));
    return NULL;
  }

  s_wordlist *res = xcalloc(sizeof(s_wordlist), 1);
  s_token *wrd = lexer_pop(lexer, errcont);
  *res = WORDLIST(TOK_STR(wrd), true, true, NULL);
  tok_free(wrd, false);
  return res;
}
