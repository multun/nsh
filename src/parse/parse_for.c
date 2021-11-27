#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/exception.h>

#include "parse.h"

static int parse_in_words(struct wordlist *target, struct lexer *lexer)
{
    int rc;
    while (true) {
        const struct token *tok;
        if ((rc = lexer_peek(&tok, lexer)))
            return rc;

        if (!tok_is(tok, TOK_WORD))
            return NSH_OK;

        struct shword *word;
        if ((rc = parse_word(&word, lexer)))
            return rc;
        wordlist_push(target, word);
    }
}

int parse_for(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_FOR, lexer)))
        return rc;

    /* Parse the variable */
    struct shast_for *for_node = shast_for_attach(res, lexer);
    if ((rc = parse_word(&for_node->var, lexer)))
        return rc;

    if ((rc = parse_newlines(lexer)))
        return rc;

    /* Parse the optional in group */
    if ((rc = parser_consume_optional(lexer, TOK_IN)) < 0)
        return rc;
    if (rc == NSH_OK) {
        if ((rc = parse_in_words(&for_node->collection, lexer)))
            return rc;
        if ((rc = parse_newlines(lexer)))
            return rc;
    }

    /* Parse ; \n* */
    if ((rc = parser_consume_optional(lexer, TOK_SEMI)) < 0)
        return rc;
    if ((rc = parse_newlines(lexer)))
        return rc;

    /* Parse the loop body */
    return parse_do_group(&for_node->body, lexer);
}
