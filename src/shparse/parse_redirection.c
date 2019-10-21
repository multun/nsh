#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "shparse/parse.h"

// this operation is atomic, no need to pass the target as argument
static void negate_ast(struct ast **ast, struct lexer *lexer, bool neg)
{
    if (!neg)
        return;

    struct ast *negation = ast_create(SHNODE_BOOL_OP, lexer);
    negation->data.ast_bool_op = ABOOL_OP(BOOL_NOT, *ast, NULL);
    *ast = negation;
}

static void pipeline_loop(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_PIPE)) {
        tok_free(lexer_pop(lexer, errcont), true);
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        struct ast *pipe = ast_create(SHNODE_PIPE, lexer);
        pipe->data.ast_pipe = APIPE(*res, NULL);
        *res = pipe;
        parse_command(&pipe->data.ast_pipe.right, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
}

void parse_pipeline(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    bool negation = tok_is(tok, TOK_BANG);
    if (negation)
        tok_free(lexer_pop(lexer, errcont), true);

    parse_command(res, lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    pipeline_loop(res, lexer, errcont);
    negate_ast(res, lexer, negation);
}

static enum redir_type parse_redir_type(const struct token *tok)
{
    if (tok_is(tok, TOK_LESS))
        return REDIR_LESS;
    if (tok_is(tok, TOK_DLESS))
        return REDIR_DLESS;
    if (tok_is(tok, TOK_GREAT))
        return REDIR_GREAT;
    if (tok_is(tok, TOK_DGREAT))
        return REDIR_DGREAT;
    if (tok_is(tok, TOK_LESSAND))
        return REDIR_LESSAND;
    if (tok_is(tok, TOK_GREATAND))
        return REDIR_GREATAND;
    if (tok_is(tok, TOK_LESSDASH))
        return REDIR_LESSDASH;
    if (tok_is(tok, TOK_LESSGREAT))
        return REDIR_LESSGREAT;
    if (tok_is(tok, TOK_CLOBBER))
        return REDIR_CLOBBER;
    abort(); // TODO: raise exception
}

void parse_redirection(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    struct token *tok = lexer_pop(lexer, errcont);

    // TODO: alloc later to avoid potential leak on exception
    *res = ast_create(SHNODE_REDIRECTION, lexer);
    int left = -1;
    if (tok_is(tok, TOK_IO_NUMBER)) {
        left = atoi(tok_buf(tok));
        tok_free(tok, true);
        tok = lexer_pop(lexer, errcont);
    }

    enum redir_type type = parse_redir_type(tok);
    tok_free(tok, true);
    (*res)->data.ast_redirection = AREDIRECTION(type, left, NULL, NULL);
    (*res)->data.ast_redirection.right = parse_word(lexer, errcont);
}
