#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>
#include <assert.h>

#include "parse.h"


static int parse_subshell(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_LPAR, lexer)))
        return rc;

    struct shast_subshell *subshell = shast_subshell_attach(res, lexer);
    if ((rc = parse_compound_list(&subshell->action, lexer)))
        return rc;

    return parser_consume(lexer, TOK_RPAR);
}

static int parse_block(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_LBRACE, lexer)))
        return rc;

    if ((rc = parse_compound_list(res, lexer)))
        return rc;

    return parser_consume(lexer, TOK_RBRACE);
}


int (*compound_command_parsers[])(struct shast **res, struct lexer *lexer) = {
    parse_subshell, parse_block, parse_if,   parse_for,
    parse_while,    parse_until, parse_case,
};


static int parse_compound_command(struct shast **res, struct lexer *lexer)
{
    int rc;
    for (size_t i = 0; i < ARR_SIZE(compound_command_parsers); i++) {
        if ((rc = compound_command_parsers[i](res, lexer)) < 0)
            return rc;
        if (rc == PARSER_NOMATCH)
            continue;
        assert(rc == NSH_OK);
        return parse_redirections(NULL, &res, lexer);
    }
    return PARSER_NOMATCH;
}

int parse_command(struct shast **res, struct lexer *lexer)
{
    int rc;

    /* Continue parsing if the command didn't fail nor succeed */
    if ((rc = parse_compound_command(res, lexer)) <= 0)
        return rc;

    /* Continue parsing if the command didn't fail nor succeed */
    if ((rc = parse_funcdec(res, lexer)) <= 0)
        return rc;

    return parse_simple_command(res, lexer);
}

int parse_funcdec(struct shast **res, struct lexer *lexer)
{
    int rc;
    const struct token *tok;

    /* The next token must be a word */
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;
    if (!token_is(tok, TOK_WORD))
        return PARSER_NOMATCH;

    /* The one after must be a ( */
    const struct token *paren_tok;
    if ((rc = lexer_peek_at(&paren_tok, lexer, 1)))
        return rc;
    if (!token_is(paren_tok, TOK_LPAR))
        return PARSER_NOMATCH;

    struct token *function_name;
    if ((rc = lexer_pop(&function_name, lexer)))
        return rc;

    struct shast_function *func = shast_function_attach(res, lexer);
    hashmap_item_init(&func->hash, token_deconstruct(function_name));

    /* Get rid of the parenthesis */
    if ((rc = parser_consume(lexer, TOK_LPAR)))
        return rc;
    if ((rc = parser_consume(lexer, TOK_RPAR)))
        return rc;

    if ((rc = parse_newlines(lexer)))
        return rc;

    if ((rc = parse_compound_command(&func->body, lexer)))
        return rc;

    struct shast **ast = &func->body;
    return parse_redirections(NULL, &ast, lexer);
}
