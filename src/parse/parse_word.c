#include <nsh_utils/alloc.h>
#include <nsh_parse/parse.h>
#include <string.h>

#include "parse.h"

int parse_word(struct shword **res, struct lexer *lexer)
{
    int rc;

    {
        const struct token *tok;
        if ((rc = lexer_peek(&tok, lexer)))
            return rc;

        if (!tok_is(tok, TOK_WORD))
            return parser_err(tok, "unexpected token %s, expected WORD", TOKT_STR(tok));
    }

    struct token *tok;
    if ((rc = lexer_pop(&tok, lexer)))
        return rc;

    struct shword *word = xmalloc(sizeof(*word) + tok_size(tok) + /* \0 */ 1);
    word->line_info = tok->lineinfo;
    strcpy(shword_buf(word), tok_buf(tok));
    tok_free(tok, true);
    *res = word;
    return NSH_OK;
}
