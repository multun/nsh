#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>

#include "parse.h"


nsh_err_t parser_err(const struct token *tok, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    lineinfo_vwarn(&tok->lineinfo, fmt, ap);

    va_end(ap);

    return NSH_PARSER_ERROR;
}


nsh_err_t parser_consume(struct lexer *lexer, enum token_type type)
{
    nsh_err_t err;
    const struct token *tok;
    if ((err = lexer_peek(&tok, lexer)))
        return err;

    if (token_is(tok, type))
        return lexer_discard(lexer);

    return parser_err(tok, "unexpected token %s, expected %s", token_buf(tok),
                      token_type_repr(type));
}

int parser_consume_optional(struct lexer *lexer, enum token_type type)
{
    int rc;
    const struct token *tok;
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;

    if (token_is(tok, type))
        return lexer_discard(lexer);

    return PARSER_NOMATCH;
}


nsh_err_t parse_newlines(struct lexer *lexer)
{
    nsh_err_t err;
    while (true) {
        const struct token *tok;
        if ((err = lexer_peek(&tok, lexer)))
            return err;
        if (!token_is(tok, TOK_NEWLINE))
            return NSH_OK;
        if ((err = lexer_discard(lexer)))
            return err;
    }
}

nsh_err_t parse(struct shast **res, struct lexer *lexer)
{
    nsh_err_t err;
    const struct token *tok;

    *res = NULL;

    // return imediatly if the first token is a newline or EOF
    if ((err = lexer_peek(&tok, lexer)))
        return err;
    if (token_is(tok, TOK_EOF) || token_is(tok, TOK_NEWLINE))
        return NSH_OK;

    // parse the rest of the grammar
    if ((err = parse_list(res, lexer)))
        return err;

    // expect a newline or eof after parsing the main list
    if ((err = lexer_peek(&tok, lexer)))
        return err;
    if (token_is(tok, TOK_EOF) || token_is(tok, TOK_NEWLINE))
        return NSH_OK;

    // otherwise, return
    return parser_err(tok, "unexpected token %s, expected 'EOF' or '\\n'",
                      token_type_repr(tok->type));
}

nsh_err_t parse_and_or(struct shast **res, struct lexer *lexer)
{
    nsh_err_t err;
    const struct token *tok;

    while (true) {
        if ((err = parse_pipeline(res, lexer)))
            return err;

        // if the next token is a && or ||, continue parsing here
        if (((err = lexer_peek(&tok, lexer))))
            return err;
        if (!token_is(tok, TOK_OR_IF) && !token_is(tok, TOK_AND_IF))
            break;
        bool is_or = token_is(tok, TOK_OR_IF);
        if ((err = lexer_discard(lexer)))
            return err;

        if ((err = parse_newlines(lexer)))
            return err;

        // create a new node and attach everything to its right child
        struct shast *left = *res;
        struct shast_bool_op *op = shast_bool_op_attach(res, lexer);
        op->type = is_or ? BOOL_OR : BOOL_AND;
        op->left = left;
        res = &op->right;
    }
    return NSH_OK;
}
