#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_newlines(s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_NEWLINE)) {
        tok_free(lexer_pop(lexer, errcont), true);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return; // the field is already initialized to NULL
    parse_list(res, lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return;
    PARSER_ERROR(&tok->lineinfo, errcont, "unxpected token %s, expected 'EOF' or '\\n'",
                 TOKT_STR(tok));
}

// TODO: store the next node in an attached point
static void list_loop(s_lexer *lexer, s_errcont *errcont, s_ast *res, const s_token *tok)
{
    s_alist *tmp = &res->data.ast_list;
    while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND)) {
        tok_free(lexer_pop(lexer, errcont), true);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
            return;
        s_alist *next = xcalloc(sizeof(s_alist), 1);
        tmp->next = next;
        tmp = next;
        *next = ALIST(NULL, NULL);
        parse_and_or(&next->action, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse_list(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    *res = xcalloc(sizeof(s_ast), 1);
    (*res)->type = SHNODE_LIST;
    parse_and_or(&(*res)->data.ast_list.action, lexer, errcont);
    const s_token *tok = lexer_peek(lexer, errcont);
    list_loop(lexer, errcont, *res, tok); // TODO: fix list_loop API
}

// TODO: check for exception leeks
void parse_and_or(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    parse_pipeline(res, lexer, errcont);
    const s_token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_OR_IF) || tok_is(tok, TOK_AND_IF)) {
        bool or = tok_is(tok, TOK_OR_IF);
        tok_free(lexer_pop(lexer, errcont), true);
        parse_newlines(lexer, errcont);
        s_ast *new_tree = xcalloc(sizeof(s_ast), 1);
        new_tree->type = SHNODE_BOOL_OP;
        new_tree->data.ast_bool_op = ABOOL_OP(or ? BOOL_OR : BOOL_AND, *res, NULL);
        *res = new_tree;
        parse_pipeline(&(*res)->data.ast_bool_op.right, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}
