#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/exception.h>

#include "parse.h"


int parse_if(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_IF, lexer)))
        return rc;

    /* Parse condition; then branch_true; */
    struct shast_if *if_node = shast_if_attach(res, lexer);
    if ((rc = parse_compound_list(&if_node->condition, lexer)))
        return rc;
    if ((rc = parser_consume(lexer, TOK_THEN)))
        return rc;
    if ((rc = parse_compound_list(&if_node->branch_true, lexer)))
        return rc;

    /* Parse elif's */
    while (true) {
        if ((rc = parser_consume_optional(lexer, TOK_ELIF)) < 0)
            return rc;
        if (rc == PARSER_NOMATCH)
            break;

        struct shast_if *elif = shast_if_attach(&if_node->branch_false, lexer);
        if ((rc = parse_compound_list(&elif->condition, lexer)))
            return rc;
        if ((rc = parser_consume(lexer, TOK_THEN)))
            return rc;
        if ((rc = parse_compound_list(&elif->branch_true, lexer)))
            return rc;
        if_node = elif;
    }

    /* Parse the else */
    if ((rc = parser_consume_optional(lexer, TOK_ELSE)) < 0)
        return rc;
    if (rc == NSH_OK) {
        if ((rc = parse_compound_list(&if_node->branch_false, lexer)))
            return rc;
    }

    return parser_consume(lexer, TOK_FI);
}
