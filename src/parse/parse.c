#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>

void parse_newlines(struct lexer *lexer, struct exception_catcher *catcher)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    while (tok_is(tok, TOK_NEWLINE)) {
        tok_free(lexer_pop(lexer, catcher), true);
        tok = lexer_peek(lexer, catcher);
    }
}

void parse(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return; // the field is already initialized to NULL
    parse_list(res, lexer, catcher);
    tok = lexer_peek(lexer, catcher);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return;
    parser_err(&tok->lineinfo, catcher, "unexpected token %s, expected 'EOF' or '\\n'",
               TOKT_STR(tok));
}

void parse_and_or(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    while (true) {
        parse_pipeline(res, lexer, catcher);
        const struct token *tok = lexer_peek(lexer, catcher);
        if (!tok_is(tok, TOK_OR_IF) && !tok_is(tok, TOK_AND_IF))
            break;
        bool is_or = tok_is(tok, TOK_OR_IF);
        tok_free(lexer_pop(lexer, catcher), true);
        parse_newlines(lexer, catcher);

        struct shast *left = *res;
        struct shast_bool_op *op = shast_bool_op_attach(res, lexer);
        op->type = is_or ? BOOL_OR : BOOL_AND;
        op->left = left;
        res = &op->right;
    }
}
