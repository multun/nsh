#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "shparse/parse.h"

static void negate_ast(struct shast **ast, struct lexer *lexer, bool neg)
{
    if (!neg)
        return;

    struct shast_negate *neg_ast = shast_negate_create(lexer);
    neg_ast->child = *ast;
    *ast = &neg_ast->base;
}

void parse_pipeline(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    /* detect pipeline negation: '! foo | bar' */
    const struct token *tok = lexer_peek(lexer, errcont);
    bool negation = tok_is(tok, TOK_BANG);
    if (negation)
        lexer_discard(lexer, errcont);

    /* parse a single command */
    parse_command(res, lexer, errcont);

    /* if the command isn't followed by a pipe, apply negation and stop there */
    tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_PIPE)) {
        negate_ast(res, lexer, negation);
        return;
    }

    /* otherwise, create a fully fledged pipeline */
    struct shast *first_child = *res;
    struct shast_pipeline *pipeline = shast_pipeline_attach(res, lexer);
    shast_vect_push(&pipeline->children, first_child);
    negate_ast(res, lexer, negation);

    /* fill the pipeline while there are '|' tokens */
    do {
        /* drop the '|' token*/
        tok_free(lexer_pop(lexer, errcont), true);
        /* drop newlines */
        parse_newlines(lexer, errcont);
        /* parse the next command in a new slot at the end of the children list */
        parse_command(shast_vect_tail_slot(&pipeline->children), lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    } while (tok_is(tok, TOK_PIPE));
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
    return REDIR_NONE;
}

int parse_redirection(struct redir_vect *vect, struct lexer *lexer, struct errcont *errcont)
{
    struct token *tok = lexer_peek(lexer, errcont);
    int left = -1;
    if (tok_is(tok, TOK_IO_NUMBER)) {
        left = atoi(tok_buf(tok));
        lexer_discard(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }

    enum redir_type type = parse_redir_type(tok);
    if (type == REDIR_NONE)
    {
        assert(left == -1);
        return 1;
    }

    struct shast_redirection *redir = zalloc(sizeof(*redir));
    redir_vect_push(vect, redir);
    redir->left = left;
    redir->type = type;
    lexer_discard(lexer, errcont);
    redir->right = parse_word(lexer, errcont);
    return 0;
}
