#include <assert.h>
#include <string.h>

#include "shparse/parse.h"
#include "utils/alloc.h"
#include "shlex/print.h"

bool start_redir(const struct token *tok)
{
    return tok_is(tok, TOK_IO_NUMBER) || tok_is(tok, TOK_DLESS) || tok_is(tok, TOK_DGREAT)
        || tok_is(tok, TOK_LESSAND) || tok_is(tok, TOK_GREATAND)
        || tok_is(tok, TOK_LESSGREAT) || tok_is(tok, TOK_LESSDASH)
        || tok_is(tok, TOK_CLOBBER) || tok_is(tok, TOK_LESS) || tok_is(tok, TOK_GREAT);
}

static void parse_assignment(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    *res = ast_create(SHNODE_ASSIGNMENT, lexer);
    struct token *tok = lexer_pop(lexer, errcont);
    char *val = strchr(tok_buf(tok), '=');

    *val = '\0';
    val++;
    (*res)->data.ast_assignment = AASSIGNMENT(tok_buf(tok), val, NULL);
    tok_free(tok, false);
}

// TODO: fix obsolete architecture
struct block_builder
{
    struct ablock *block;
    struct ast *assign;
    struct ast *redir;
    struct wordlist *elm;
};

static bool loop_redir(struct lexer *lexer, struct errcont *errcont, struct block_builder *build)
{
    struct ast **target;
    if (build->redir)
        target = &build->redir->data.ast_redirection.action;
    else
        target = &build->block->redir;
    parse_redirection(target, lexer, errcont);
    build->redir = *target;
    return true;
}

static bool prefix_loop(struct lexer *lexer, struct errcont *errcont, struct block_builder *build)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_ASSIGNMENT_WORD) || start_redir(tok)) {
        if (tok_is(tok, TOK_ASSIGNMENT_WORD)) {
            struct ast **target;
            if (build->assign)
                target = &build->assign->data.ast_assignment.action;
            else
                target = &build->block->def;
            parse_assignment(target, lexer, errcont);
            build->assign = *target;
        } else if (!loop_redir(lexer, errcont, build))
            return false;
        tok = lexer_peek(lexer, errcont);
    }
    return true;
}

static bool element_loop(struct lexer *lexer, struct errcont *errcont, struct block_builder *build)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    while (tok_is(tok, TOK_WORD) || start_redir(tok)) {
        if (tok_is(tok, TOK_WORD)) {
            struct wordlist **target;
            if (build->elm)
                target = &build->elm->next;
            else {
                build->block->cmd = ast_create(SHNODE_CMD, lexer);
                target = &build->block->cmd->data.ast_cmd.wordlist;
            }
            parse_word(target, lexer, errcont);
            build->elm = *target;
        } else if (!loop_redir(lexer, errcont, build))
            return false;
        tok = lexer_peek(lexer, errcont);
    }
    return true;
}

void parse_simple_command(struct ast **res, struct lexer *lexer, struct errcont *errcont)
{
    *res = ast_create(SHNODE_BLOCK, lexer);
    (*res)->data.ast_block = ABLOCK(NULL, NULL, NULL);
    struct block_builder builder = {
        .block = &(*res)->data.ast_block,
        .assign = NULL,
        .redir = NULL,
        .elm = NULL,
    };

    if (!prefix_loop(lexer, errcont, &builder) || !element_loop(lexer, errcont, &builder))
        return;

    if (!(*res)->data.ast_block.redir && !(*res)->data.ast_block.def
        && !(*res)->data.ast_block.cmd) {
        const struct token *tok = lexer_peek(lexer, errcont);
        PARSER_ERROR(&tok->lineinfo, errcont, "parsing error %s", TOKT_STR(tok));
    }
}
