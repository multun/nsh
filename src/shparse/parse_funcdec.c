#include "shparse/parse.h"
#include "shlex/print.h"

static bool parse_func_remove_par(struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_LPAR))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected '('",
                     TOKT_STR(tok));
    tok_free(lexer_pop(lexer, errcont), true);
    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_RPAR))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected '('",
                     TOKT_STR(tok));
    return true;
}

void parse_funcdec(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    *res = ast_create(SHNODE_FUNCTION, lexer);

    struct token *word = lexer_pop(lexer, errcont);
    (*res)->data.ast_function.name = tok_buf(word);
    tok_free(word, false);
    if (!parse_func_remove_par(lexer, errcont))
        return;

    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    parse_shell_command(&(*res)->data.ast_function.value, lexer, errcont);
}
