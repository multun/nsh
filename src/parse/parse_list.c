#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>

#include "parse.h"

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


nsh_err_t parse_compound_list(struct shast **res, struct lexer *lexer)
{
    nsh_err_t err;

    /* start by parsing a pipeline, and stopping right away if we can */
    if ((err = parse_newlines(lexer)))
        return err;

    struct shast_list *list = NULL;
    while (true) {
        if ((err = parse_and_or(res, lexer)))
            return err;

        /* stop if there's no command separator */
        const struct token *tok;
        if ((err = lexer_peek(&tok, lexer)))
            return err;

        if (tok_is(tok, TOK_AND)) {
            /* mark the last ast as asynchronous */
            (*res)->async = true;
        } else if (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
            /* stop parsing the list if an unexpected token is met */
            break;

        if ((err = lexer_discard(lexer)))
            return err;

        if ((err = parse_newlines(lexer)))
            return err;

        /* stop if there's a compound list terminator keyword */
        if ((err = lexer_peek(&tok, lexer)))
            return err;
        if (compound_list_block_end(tok))
            break;

        /* if the list wasn't started, create it */
        if (list == NULL) {
            struct shast *first_child = *res;
            list = shast_list_attach(res, lexer);
            shast_vect_push(&list->commands, first_child);
        }
        res = shast_vect_tail_slot(&list->commands);
    }
    return NSH_OK;
}


nsh_err_t parse_list(struct shast **res, struct lexer *lexer)
{
    nsh_err_t err;

    struct shast_list *list = NULL;
    while (true) {
        /* start by parsing a pipeline, and stopping right away if we can */
        if ((err = parse_and_or(res, lexer)))
            return err;

        /* handle parsing of ; and & */
        const struct token *tok;
        if ((err = lexer_peek(&tok, lexer)))
            return err;
        if (tok_is(tok, TOK_AND))
            (*res)->async = true;
        if (!tok_is(tok, TOK_AND) && !tok_is(tok, TOK_SEMI))
            break;
        if ((err = lexer_discard(lexer)))
            return err;
        if ((err = lexer_peek(&tok, lexer)))
            return err;

        /* stop when:
        **  - COMMAND ; EOF
        **  - COMMAND ; NEWLINE
        */
        if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
            break;

        /* if the list wasn't started, create it */
        if (list == NULL) {
            struct shast *first_child = *res;
            list = shast_list_attach(res, lexer);
            shast_vect_push(&list->commands, first_child);
        }
        res = shast_vect_tail_slot(&list->commands);
    }
    return NSH_OK;
}
