#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

static void rule_case(struct lexer *lexer, struct errcont *errcont, struct ast *res)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_IN))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected 'in'",
                     TOKT_STR(tok));
    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    parse_case_clause(&res->data.ast_case.nodes, lexer, errcont);
    tok_free(lexer_pop(lexer, errcont), true);
}

void parse_rule_case(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    tok_free(lexer_pop(lexer, errcont), true);
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_WORD))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected WORD",
                     TOKT_STR(tok));
    *res = ast_create(SHNODE_CASE, lexer);
    parse_word(&(*res)->data.ast_case.var, lexer, errcont);

    parse_newlines(lexer, errcont);
    rule_case(lexer, errcont, *res);
}

static void case_clause_loop(struct lexer *lexer, struct errcont *errcont, struct acase_node *tail)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    for (; tok_is(tok, TOK_DSEMI); tail = tail->next) {
        tok_free(lexer_pop(lexer, errcont), true);
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_ESAC))
            return;
        parse_case_item(&tail->next, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse_case_clause(struct acase_node **res, struct lexer *lexer, struct errcont *errcont)
{
    parse_case_item(res, lexer, errcont);
    parse_newlines(lexer, errcont);
    case_clause_loop(lexer, errcont, *res);
}

static void parse_pattern(struct wordlist **res, struct lexer *lexer, struct errcont *errcont)
{
    parse_word(res, lexer, errcont);
    const struct token *tok = lexer_peek(lexer, errcont);
    for (; tok_is(tok, TOK_PIPE); res = &(*res)->next) {
        tok_free(lexer_pop(lexer, errcont), true);
        parse_word(&(*res)->next, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse_case_item(struct acase_node **res, struct lexer *lexer, struct errcont *errcont)
{
    *res = xcalloc(sizeof(struct acase_node), 1);
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_LPAR))
        tok_free(lexer_pop(lexer, errcont), true);
    parse_pattern(&(*res)->pattern, lexer, errcont);

    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_RPAR))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected ')'",
                     TOKT_STR(tok));
    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_ESAC) && !tok_is(tok, TOK_DSEMI))
        parse_compound_list(&(*res)->action, lexer, errcont);
}
