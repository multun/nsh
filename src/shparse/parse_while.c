#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_rule_while(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    tok_free(lexer_pop(lexer, errcont), true);
    *res = xcalloc(sizeof(s_ast), 1);
    (*res)->type = SHNODE_WHILE;
    parse_compound_list(&(*res)->data.ast_while.condition, lexer, errcont);
    parse_do_group(&(*res)->data.ast_while.actions, lexer, errcont);
}

void parse_rule_until(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    tok_free(lexer_pop(lexer, errcont), true);
    *res = xcalloc(sizeof(s_ast), 1);
    (*res)->type = SHNODE_UNTIL;
    parse_compound_list(&(*res)->data.ast_until.condition, lexer, errcont);
    parse_do_group(&(*res)->data.ast_until.actions, lexer, errcont);
}

void parse_do_group(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DO))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected 'do'",
                     TOKT_STR(tok));
    tok_free(lexer_pop(lexer, errcont), true);
    parse_compound_list(res, lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DONE))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected 'done'",
                     TOKT_STR(tok));
    tok_free(lexer_pop(lexer, errcont), true);
}
