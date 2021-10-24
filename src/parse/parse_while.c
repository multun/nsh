#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>

void parse_rule_while(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    lexer_discard(lexer, ex_scope);
    struct shast_while *while_node = shast_while_attach(res, lexer);
    while_node->is_until = false;
    parse_compound_list(&while_node->condition, lexer, ex_scope);
    parse_do_group(&while_node->body, lexer, ex_scope);
}

void parse_rule_until(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    lexer_discard(lexer, ex_scope);
    struct shast_while *until_node = shast_while_attach(res, lexer);
    until_node->is_until = true;
    parse_compound_list(&until_node->condition, lexer, ex_scope);
    parse_do_group(&until_node->body, lexer, ex_scope);
}

void parse_do_group(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    parser_consume(lexer, TOK_DO, ex_scope);
    parse_compound_list(res, lexer, ex_scope);
    parser_consume(lexer, TOK_DONE, ex_scope);
}
