#include <nsh_utils/alloc.h>
#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <string.h>

struct shword *parse_word(struct lexer *lexer, struct exception_catcher *catcher)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    if (!tok_is(tok, TOK_WORD))
        parser_err(&tok->lineinfo, catcher, "unexpected token %s, expected WORD",
                   TOKT_STR(tok));

    struct token *word = lexer_pop(lexer, catcher);
    struct shword *res = xmalloc(sizeof(*res) + tok_size(tok) + /* \0 */ 1);
    res->line_info = tok->lineinfo;
    strcpy(shword_buf(res), tok_buf(word));
    tok_free(word, true);
    return res;
}
