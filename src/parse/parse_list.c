#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>


static bool compound_list_block_end(const struct token *tok)
{
    switch (tok->type) {
    case TOK_DO:
    case TOK_DONE:
    case TOK_DSEMI:
    case TOK_ELIF:
    case TOK_ELSE:
    case TOK_ESAC:
    case TOK_FI:
    case TOK_RBRACE:
    case TOK_RPAR:
    case TOK_THEN:
        return true;
    default:
        return false;
    }
}


static bool parse_compound_list_end(struct shast *prev_ast, struct lexer *lexer, struct exception_catcher *catcher)
{
    /* stop if there's no command separator */
    const struct token *tok = lexer_peek(lexer, catcher);

    if (tok_is(tok, TOK_AND)) {
        /* mark the last ast as asynchronous */
        prev_ast->async = true;
    } else if (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
        /* stop parsing the list if an unexpected token is met */
        return true;

    lexer_discard(lexer, catcher);
    parse_newlines(lexer, catcher);

    /* stop if there's a compound list terminator keyword */
    tok = lexer_peek(lexer, catcher);
    return compound_list_block_end(tok);
}


void parse_compound_list(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    /* start by parsing a pipeline, and stopping right away if we can */
    parse_newlines(lexer, catcher);
    parse_and_or(res, lexer, catcher);
    if (parse_compound_list_end(*res, lexer, catcher))
        return;

    /* if the list doesn't end there, inject a list node */
    struct shast *first_child = *res;
    struct shast_list *list = shast_list_attach(res, lexer);
    shast_vect_push(&list->commands, first_child);

    while (true) {
        struct shast **last_ast = shast_vect_tail_slot(&list->commands);
        parse_and_or(last_ast, lexer, catcher);
        if (parse_compound_list_end(*last_ast, lexer, catcher))
            break;
    }
}


static bool parse_list_end(struct shast *prev_ast, struct lexer *lexer, struct exception_catcher *catcher)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    if (tok_is(tok, TOK_AND)) {
        /* mark the last ast as asynchronous */
        prev_ast->async = true;
    } else if (tok_is(tok, TOK_SEMI)) {
        /* do nothing */
    } else
        /* stop parsing the list */
        return true;

    lexer_discard(lexer, catcher);

    /* stop when:
    **  - COMMAND ; EOF
    **  - COMMAND ; NEWLINE
    */
    tok = lexer_peek(lexer, catcher);
    return tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE);
}


void parse_list(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    /* start by parsing a pipeline, and stopping right away if we can */
    parse_and_or(res, lexer, catcher);
    if (parse_list_end(*res, lexer, catcher))
        return;

    /* if the list doesn't end there, inject a list node */
    struct shast *first_child = *res;
    struct shast_list *list = shast_list_attach(res, lexer);
    shast_vect_push(&list->commands, first_child);

    while (true) {
        struct shast **last_ast = shast_vect_tail_slot(&list->commands);
        parse_and_or(last_ast, lexer, catcher);
        if (parse_list_end(*last_ast, lexer, catcher))
            break;
    }
}
