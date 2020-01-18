#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static bool compound_list_end(const struct token *tok)
{
    /* TODO: make a lookup table */
    return tok_is(tok, TOK_THEN) || tok_is(tok, TOK_ELSE) || tok_is(tok, TOK_FI)
        || tok_is(tok, TOK_DONE) || tok_is(tok, TOK_ESAC) || tok_is(tok, TOK_ELIF)
        || tok_is(tok, TOK_DSEMI) || tok_is(tok, TOK_DO) || tok_is(tok, TOK_RBRACE)
        || tok_is(tok, TOK_RPAR);
}

void parse_compound_list(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    parse_newlines(lexer, errcont);
    struct shast_list *list = shast_list_attach(res, lexer);
    while (true) {
        // parse the command
        shast_vect_push(&list->commands, NULL);
        struct shast **last_ast = shast_vect_last(&list->commands);
        parse_and_or(last_ast, lexer, errcont);

        // stop if there's no command separator
        const struct token *tok = lexer_peek(lexer, errcont);

        if (tok_is(tok, TOK_AND)) {
            /* mark the last ast as asynchronous */
            (*last_ast)->async = true;
        } else if (tok_is(tok, TOK_SEMI)) {
            /* do nothing */
        } else
            /* stop parsing the list */
            break;

        lexer_discard(lexer, errcont);
        parse_newlines(lexer, errcont);

        // stop if there's a compound list terminator keyword
        tok = lexer_peek(lexer, errcont);
        if (compound_list_end(tok))
            break;
    }
}
