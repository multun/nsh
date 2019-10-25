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

void parse(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return; // the field is already initialized to NULL
    parse_list(res, lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
        return;
    parser_err(&tok->lineinfo, errcont, "unxpected token %s, expected 'EOF' or '\\n'",
               TOKT_STR(tok));
}

void parse_list(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    struct shast_list *list = shast_list_attach(res, lexer);
    while (true) {
        shast_vect_push(&list->commands, NULL);
        parse_and_or(shast_vect_last(&list->commands), lexer, errcont);
        const struct token *tok = lexer_peek(lexer, errcont);
        if (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_AND))
            break;
        lexer_discard(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
            return;
    }
}

void parse_and_or(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    while (true) {
        parse_pipeline(res, lexer, errcont);
        const struct token *tok = lexer_peek(lexer, errcont);
        if (!tok_is(tok, TOK_OR_IF) && !tok_is(tok, TOK_AND_IF))
            break;
        bool is_or = tok_is(tok, TOK_OR_IF);
        tok_free(lexer_pop(lexer, errcont), true);
        parse_newlines(lexer, errcont);

        struct shast *left = *res;
        struct shast_bool_op *op = shast_bool_op_attach(res, lexer);
        op->type = is_or ? BOOL_OR : BOOL_AND;
        op->left = left;
        res = &op->right;
    }
}
