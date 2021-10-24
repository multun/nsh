#include <assert.h>
#include <string.h>

#include <nsh_parse/parse.h>
#include <nsh_utils/alloc.h>
#include <nsh_lex/print.h>

static void parse_assignment(struct assign_vect *vect, struct lexer *lexer, struct exception_catcher *catcher)
{
    struct shast_assignment *assign = zalloc(sizeof(*assign));
    assign->line_info = *lexer_line_info(lexer);
    struct token *tok = lexer_pop(lexer, catcher);
    char *val = strchr(tok_buf(tok), '=');
    *(val++) = '\0';
    assign->name = tok_buf(tok);
    assign->value = val;
    tok_free(tok, false);
    assign_vect_push(vect, assign);
}

static bool prefix_loop(struct lexer *lexer, struct exception_catcher *catcher, struct shast_block *block)
{
    while (true) {
        const struct token *tok = lexer_peek(lexer, catcher);
        if (tok_is(tok, TOK_ASSIGNMENT_WORD)) {
            parse_assignment(&block->assigns, lexer, catcher);
            continue;
        }

        if (parse_redirection(&block->redirs, lexer, catcher) == 0)
            continue;
        break;
    }
    return true;
}

static bool element_loop(struct lexer *lexer, struct exception_catcher *catcher,
                         struct shast_block *block, struct shast_cmd **cmd)
{
    while (true) {
        const struct token *tok = lexer_peek(lexer, catcher);
        if (tok_is(tok, TOK_WORD)) {
            // initialize the command node if missing
            if (*cmd == NULL)
                *cmd = shast_cmd_attach(&block->command, lexer);

            wordlist_push(&(*cmd)->arguments, parse_word(lexer, catcher));
            continue;
        }
        if (parse_redirection(&block->redirs, lexer, catcher) == 0)
            continue;
        break;
    }
    return true;
}

void parse_simple_command(struct shast **res, struct lexer *lexer, struct exception_catcher *catcher)
{
    struct shast_block *block = shast_block_attach(res, lexer);
    struct shast_cmd *cmd = NULL;

    if (!prefix_loop(lexer, catcher, block))
        return;

    if (!element_loop(lexer, catcher, block, &cmd))
        return;

    if (redir_vect_size(&block->redirs) == 0
        && assign_vect_size(&block->assigns) == 0
        && cmd == NULL) {
        const struct token *tok = lexer_peek(lexer, catcher);
        parser_err(&tok->lineinfo, catcher, "parsing error %s", TOKT_STR(tok));
    }
}
