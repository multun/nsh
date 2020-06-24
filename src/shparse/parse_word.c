#include "utils/alloc.h"
#include "shparse/parse.h"
#include "shlex/print.h"
#include <string.h>

struct shword *parse_word(struct lexer *lexer, struct ex_scope *ex_scope)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (!tok_is(tok, TOK_WORD))
        parser_err(&tok->lineinfo, ex_scope, "unexpected token %s, expected WORD",
                   TOKT_STR(tok));

    struct token *word = lexer_pop(lexer, ex_scope);
    struct shword *res = xmalloc(sizeof(*res) + tok_size(tok) + /* \0 */ 1);
    res->line_info = tok->lineinfo;
    strcpy(shword_buf(res), tok_buf(word));
    tok_free(word, true);
    return res;
}
