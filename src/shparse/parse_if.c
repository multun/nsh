#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/error.h"


void parse_rule_if(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    // consume the if
    lexer_discard(lexer, errcont);
    struct shast_if *if_node = shast_if_attach(res, lexer);
    parse_compound_list(&if_node->condition, lexer, errcont);
    parser_consume(lexer, TOK_THEN, errcont);
    parse_compound_list(&if_node->branch_true, lexer, errcont);
    const struct token *tok = lexer_peek(lexer, errcont);

    // elif case
    while (true) {
        tok = lexer_peek(lexer, errcont);
        if (!tok_is(tok, TOK_ELIF))
            break;
        lexer_discard(lexer, errcont);
        struct shast_if *elif = shast_if_attach(&if_node->branch_false, lexer);
        parse_compound_list(&elif->condition, lexer, errcont);
        parser_consume(lexer, TOK_THEN, errcont);
        parse_compound_list(&elif->branch_true, lexer, errcont);
        if_node = elif;
    }

    // else case
    if (tok_is(tok, TOK_ELSE))
    {
        lexer_discard(lexer, errcont);
        parse_compound_list(&if_node->branch_false, lexer, errcont);
    }

    parser_consume(lexer, TOK_FI, errcont);
}
