#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"

void parse_word(s_wordlist **res, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_WORD))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected WORD",
                     TOKT_STR(tok));

    *res = xcalloc(sizeof(s_wordlist), 1);
    s_token *wrd = lexer_pop(lexer, errcont);
    **res = WORDLIST(tok_buf(wrd), true, true, NULL);
    tok_free(wrd, false);
}
