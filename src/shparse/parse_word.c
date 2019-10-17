#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"

void parse_word(struct wordlist **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_WORD))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected WORD",
                     TOKT_STR(tok));

    *res = xcalloc(sizeof(struct wordlist), 1);
    struct token *wrd = lexer_pop(lexer, errcont);
    **res = WORDLIST(tok_buf(wrd), NULL);
    tok_free(wrd, false);
}
