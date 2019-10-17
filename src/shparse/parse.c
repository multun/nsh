#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"

void parse_newlines(struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_NEWLINE)) {
        tok_free(lexer_pop(lexer, errcont), true);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
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
static void list_loop(struct lexer *lexer, struct errcont *errcont, struct ast *res, const struct token *tok)
{
    struct alist *tmp = &res->data.ast_list;
    while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND)) {
        tok_free(lexer_pop(lexer, errcont), true);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
            return;
        struct alist *next = xcalloc(sizeof(struct alist), 1);
        tmp->next = next;
        tmp = next;
        *next = ALIST(NULL, NULL);
        parse_and_or(&next->action, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse_list(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    *res = ast_create(SHNODE_LIST, lexer);
    parse_and_or(&(*res)->data.ast_list.action, lexer, errcont);
    const struct token *tok = lexer_peek(lexer, errcont);
    list_loop(lexer, errcont, *res, tok); // TODO: fix list_loop API
}

// TODO: check for exception leeks
void parse_and_or(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    parse_pipeline(res, lexer, errcont);
    const struct token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_OR_IF) || tok_is(tok, TOK_AND_IF)) {
        bool or = tok_is(tok, TOK_OR_IF);
        tok_free(lexer_pop(lexer, errcont), true);
        parse_newlines(lexer, errcont);
        struct ast *new_tree = ast_create(SHNODE_BOOL_OP, lexer);
        new_tree->data.ast_bool_op = ABOOL_OP(or ? BOOL_OR : BOOL_AND, *res, NULL);
        *res = new_tree;
        parse_pipeline(&(*res)->data.ast_bool_op.right, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}
