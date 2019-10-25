#include "shparse/parse.h"
#include "shlex/print.h"

static bool parse_func_remove_par(struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_LPAR))
        parser_err(&tok->lineinfo, errcont, "unexpected token %s, expected '('",
                   TOKT_STR(tok));
    lexer_discard(lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_RPAR))
        parser_err(&tok->lineinfo, errcont, "unexpected token %s, expected '('",
                   TOKT_STR(tok));
    return true;
}

void parse_funcdec(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    struct shast_function *func = shast_function_attach(res, lexer);
    struct token *word = lexer_pop(lexer, errcont);
    func->name = tok_buf(word);
    tok_free(word, false);
    if (!parse_func_remove_par(lexer, errcont))
        return;

    lexer_discard(lexer, errcont);
    parse_newlines(lexer, errcont);

    parser_consume(lexer, TOK_LBRACE, errcont);
    lexer_discard(lexer, errcont);
    parse_compound_list(&func->body, lexer, errcont);
    parser_consume(lexer, TOK_RBRACE, errcont);
}
