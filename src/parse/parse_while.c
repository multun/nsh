#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>

void parse_rule_while(struct shast **res, struct lexer *lexer,
                      struct exception_catcher *catcher)
{
    lexer_discard(lexer, catcher);
    struct shast_while *while_node = shast_while_attach(res, lexer);
    while_node->is_until = false;
    parse_compound_list(&while_node->condition, lexer, catcher);
    parse_do_group(&while_node->body, lexer, catcher);
}

void parse_rule_until(struct shast **res, struct lexer *lexer,
                      struct exception_catcher *catcher)
{
    lexer_discard(lexer, catcher);
    struct shast_while *until_node = shast_while_attach(res, lexer);
    until_node->is_until = true;
    parse_compound_list(&until_node->condition, lexer, catcher);
    parse_do_group(&until_node->body, lexer, catcher);
}

void parse_do_group(struct shast **res, struct lexer *lexer,
                    struct exception_catcher *catcher)
{
    parser_consume(lexer, TOK_DO, catcher);
    parse_compound_list(res, lexer, catcher);
    parser_consume(lexer, TOK_DONE, catcher);
}
