#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"


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


static bool parse_compound_list_end(struct shast *prev_ast, struct lexer *lexer, struct errcont *errcont)
{
    /* stop if there's no command separator */
    const struct token *tok = lexer_peek(lexer, errcont);

    if (tok_is(tok, TOK_AND)) {
        /* mark the last ast as asynchronous */
        prev_ast->async = true;
    } else if (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE))
        /* stop parsing the list if an unexpected token is met */
        return true;

    lexer_discard(lexer, errcont);
    parse_newlines(lexer, errcont);

    /* stop if there's a compound list terminator keyword */
    tok = lexer_peek(lexer, errcont);
    return compound_list_block_end(tok);
}


void parse_compound_list(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    /* start by parsing a pipeline, and stopping right away if we can */
    parse_newlines(lexer, errcont);
    parse_and_or(res, lexer, errcont);
    if (parse_compound_list_end(*res, lexer, errcont))
        return;

    /* if the list doesn't end there, inject a list node */
    struct shast *first_child = *res;
    struct shast_list *list = shast_list_attach(res, lexer);
    shast_vect_push(&list->commands, first_child);

    while (true) {
        struct shast **last_ast = shast_vect_tail_slot(&list->commands);
        parse_and_or(last_ast, lexer, errcont);
        if (parse_compound_list_end(*last_ast, lexer, errcont))
            break;
    }
}


static bool parse_list_end(struct shast *prev_ast, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_AND)) {
        /* mark the last ast as asynchronous */
        prev_ast->async = true;
    } else if (tok_is(tok, TOK_SEMI)) {
        /* do nothing */
    } else
        /* stop parsing the list */
        return true;

    lexer_discard(lexer, errcont);

    /* stop when:
    **  - COMMAND ; EOF
    **  - COMMAND ; NEWLINE
    */
    tok = lexer_peek(lexer, errcont);
    return tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE);
}


void parse_list(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    /* start by parsing a pipeline, and stopping right away if we can */
    parse_and_or(res, lexer, errcont);
    if (parse_list_end(*res, lexer, errcont))
        return;

    /* if the list doesn't end there, inject a list node */
    struct shast *first_child = *res;
    struct shast_list *list = shast_list_attach(res, lexer);
    shast_vect_push(&list->commands, first_child);

    while (true) {
        struct shast **last_ast = shast_vect_tail_slot(&list->commands);
        parse_and_or(last_ast, lexer, errcont);
        if (parse_list_end(*last_ast, lexer, errcont))
            break;
    }
}
