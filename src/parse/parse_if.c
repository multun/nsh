#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/exception.h>


void parse_rule_if(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    // consume the if
    lexer_discard(lexer, catcher);
    struct shast_if *if_node = shast_if_attach(res, lexer);
    parse_compound_list(&if_node->condition, lexer, catcher);
    parser_consume(lexer, TOK_THEN, catcher);
    parse_compound_list(&if_node->branch_true, lexer, catcher);
    const struct token *tok = lexer_peek(lexer, catcher);

    // elif case
    while (true) {
        tok = lexer_peek(lexer, catcher);
        if (!tok_is(tok, TOK_ELIF))
            break;
        lexer_discard(lexer, catcher);
        struct shast_if *elif = shast_if_attach(&if_node->branch_false, lexer);
        parse_compound_list(&elif->condition, lexer, catcher);
        parser_consume(lexer, TOK_THEN, catcher);
        parse_compound_list(&elif->branch_true, lexer, catcher);
        if_node = elif;
    }

    // else case
    if (tok_is(tok, TOK_ELSE))
    {
        lexer_discard(lexer, catcher);
        parse_compound_list(&if_node->branch_false, lexer, catcher);
    }

    parser_consume(lexer, TOK_FI, catcher);
}
