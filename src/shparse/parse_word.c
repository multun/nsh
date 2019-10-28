#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"
#include <string.h>

struct shword *parse_word(struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_WORD))
        parser_err(&tok->lineinfo, errcont, "unexpected token %s, expected WORD",
                   TOKT_STR(tok));

    struct token *word = lexer_pop(lexer, errcont);
    struct shword *res = xmalloc(sizeof(*res) + tok_size(tok) + /* \0 */ 1);
    res->line_info = tok->lineinfo;
    strcpy(shword_buf(res), tok_buf(word));
    tok_free(word, true);
    return res;
}
