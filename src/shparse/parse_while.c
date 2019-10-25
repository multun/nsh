#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_rule_while(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    lexer_discard(lexer, errcont);
    struct shast_while *while_node = shast_while_attach(res, lexer);
    while_node->is_until = false;
    parse_compound_list(&while_node->condition, lexer, errcont);
    parse_do_group(&while_node->body, lexer, errcont);
}

void parse_rule_until(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    lexer_discard(lexer, errcont);
    struct shast_while *until_node = shast_while_attach(res, lexer);
    until_node->is_until = true;
    parse_compound_list(&until_node->condition, lexer, errcont);
    parse_do_group(&until_node->body, lexer, errcont);
}

void parse_do_group(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    parser_consume(lexer, TOK_DO, errcont);
    parse_compound_list(res, lexer, errcont);
    parser_consume(lexer, TOK_DONE, errcont);
}
