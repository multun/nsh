#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/error.h"


void parse_rule_if(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    // consume the if
    lexer_discard(lexer, ex_scope);
    struct shast_if *if_node = shast_if_attach(res, lexer);
    parse_compound_list(&if_node->condition, lexer, ex_scope);
    parser_consume(lexer, TOK_THEN, ex_scope);
    parse_compound_list(&if_node->branch_true, lexer, ex_scope);
    const struct token *tok = lexer_peek(lexer, ex_scope);

    // elif case
    while (true) {
        tok = lexer_peek(lexer, ex_scope);
        if (!tok_is(tok, TOK_ELIF))
            break;
        lexer_discard(lexer, ex_scope);
        struct shast_if *elif = shast_if_attach(&if_node->branch_false, lexer);
        parse_compound_list(&elif->condition, lexer, ex_scope);
        parser_consume(lexer, TOK_THEN, ex_scope);
        parse_compound_list(&elif->branch_true, lexer, ex_scope);
        if_node = elif;
    }

    // else case
    if (tok_is(tok, TOK_ELSE))
    {
        lexer_discard(lexer, ex_scope);
        parse_compound_list(&if_node->branch_false, lexer, ex_scope);
    }

    parser_consume(lexer, TOK_FI, ex_scope);
}
