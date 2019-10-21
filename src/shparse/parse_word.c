#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"

char *parse_word(struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_WORD))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected WORD",
                     TOKT_STR(tok));

    struct token *word = lexer_pop(lexer, errcont);
    char *res = tok_buf(word);
    tok_free(word, false);
    return res;
}
