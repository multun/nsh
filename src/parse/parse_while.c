#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>

#include "parse.h"

int parse_while(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_WHILE, lexer)))
        return rc;

    struct shast_while *while_node = shast_while_attach(res, lexer);
    while_node->is_until = false;
    if ((rc = parse_compound_list(&while_node->condition, lexer)))
        return rc;
    return parse_do_group(&while_node->body, lexer);
}

int parse_until(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match_discard(TOK_UNTIL, lexer)))
        return rc;

    struct shast_while *until_node = shast_while_attach(res, lexer);
    until_node->is_until = true;
    if ((rc = parse_compound_list(&until_node->condition, lexer)))
        return rc;
    return parse_do_group(&until_node->body, lexer);
}

int parse_do_group(struct shast **res, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_consume(lexer, TOK_DO)))
        return rc;
    if ((rc = parse_compound_list(res, lexer)))
        return rc;
    return parser_consume(lexer, TOK_DONE);
}
