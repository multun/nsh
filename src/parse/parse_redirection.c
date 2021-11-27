#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <nsh_parse/parse.h>

#include "parse.h"


static void negate_ast(struct shast **ast, struct lexer *lexer, bool neg)
{
    if (!neg)
        return;

    struct shast_negate *neg_ast = shast_negate_create(lexer);
    neg_ast->child = *ast;
    *ast = &neg_ast->base;
}

nsh_err_t parse_pipeline(struct shast **res, struct lexer *lexer)
{
    nsh_err_t err;

    /* detect pipeline negation: '! foo | bar' */
    const struct token *tok;
    if ((err = lexer_peek(&tok, lexer)))
        return err;
    bool negation = token_is(tok, TOK_BANG);
    if (negation) {
        if ((err = lexer_discard(lexer)))
            return err;
    }

    /* parse a single command */
    if ((err = parse_command(res, lexer)))
        return err;

    /* if the command isn't followed by a pipe, apply negation and stop there */
    if ((err = lexer_peek(&tok, lexer)))
        return err;
    if (!token_is(tok, TOK_PIPE)) {
        negate_ast(res, lexer, negation);
        return NSH_OK;
    }

    /* otherwise, create a fully fledged pipeline */
    struct shast *first_child = *res;
    struct shast_pipeline *pipeline = shast_pipeline_attach(res, lexer);
    shast_vect_push(&pipeline->children, first_child);
    negate_ast(res, lexer, negation);

    /* fill the pipeline while there are '|' tokens */
    do {
        /* drop the '|' token*/
        if ((err = lexer_discard(lexer)))
            return err;
        /* drop newlines */
        if ((err = parse_newlines(lexer)))
            return err;
        /* parse the next command in a new slot at the end of the children list */
        if ((err = parse_command(shast_vect_tail_slot(&pipeline->children), lexer)))
            return err;

        if ((err = lexer_peek(&tok, lexer)))
            return err;
    } while (token_is(tok, TOK_PIPE));
    return NSH_OK;
}

static enum redir_type parse_redir_type(const struct token *tok)
{
    if (token_is(tok, TOK_LESS))
        return REDIR_LESS;
    if (token_is(tok, TOK_DLESS))
        return REDIR_DLESS;
    if (token_is(tok, TOK_GREAT))
        return REDIR_GREAT;
    if (token_is(tok, TOK_DGREAT))
        return REDIR_DGREAT;
    if (token_is(tok, TOK_LESSAND))
        return REDIR_LESSAND;
    if (token_is(tok, TOK_GREATAND))
        return REDIR_GREATAND;
    if (token_is(tok, TOK_LESSDASH))
        return REDIR_LESSDASH;
    if (token_is(tok, TOK_LESSGREAT))
        return REDIR_LESSGREAT;
    if (token_is(tok, TOK_CLOBBER))
        return REDIR_CLOBBER;
    return REDIR_NONE;
}

int parse_redirection(struct shast_redirection **res, struct lexer *lexer)
{
    int rc;

    const struct token *tok;
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;

    int left = -1;
    if (token_is(tok, TOK_IO_NUMBER)) {
        left = atoi(token_buf(tok));
        if ((rc = lexer_discard(lexer)))
            return rc;
        if ((rc = lexer_peek(&tok, lexer)))
            return rc;
    }

    enum redir_type type = parse_redir_type(tok);
    if (type == REDIR_NONE) {
        assert(left == -1);
        return PARSER_NOMATCH;
    }

    if ((rc = lexer_discard(lexer)))
        return rc;

    struct shword *right;
    if ((rc = parse_word(&right, lexer)))
        return rc;

    struct shast_redirection *redir = zalloc(sizeof(*redir));
    redir->type = type;
    redir->left = left;
    redir->right = right;
    *res = redir;
    return NSH_OK;
}

int parse_redirections(struct shast_block **block, struct shast ***res,
                       struct lexer *lexer)
{
    int rc;

    /* The block argument enables parsing redirections
       which may be at multiple spots. */
    struct shast_block *local_block = NULL;
    if (block == NULL)
        block = &local_block;

    int parsed_redirections = 0;
    while (true) {
        /* Try to parse a redirection */
        struct shast_redirection *redir;
        if ((rc = parse_redirection(&redir, lexer)) < 0)
            return rc;

        /* Succeed if the next token isn't part of a redirection */
        if (rc == PARSER_NOMATCH)
            return parsed_redirections;

        parsed_redirections++;

        /** Lazily create the redirection block */
        if (*block == NULL) {
            struct shast *old_root = **res;
            *block = shast_block_attach(*res, lexer);
            (*block)->command = old_root;
            *res = &(*block)->command;
        }
        redir_vect_push(&(*block)->redirs, redir);
    }
}
