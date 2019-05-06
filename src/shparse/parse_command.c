#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"

static bool is_first_keyword(const s_token *tok)
{
    return tok_is(tok, TOK_IF) || tok_is(tok, TOK_FOR) || tok_is(tok, TOK_WHILE)
        || tok_is(tok, TOK_UNTIL) || tok_is(tok, TOK_CASE);
}

static s_ast *redirection_loop_sec(s_lexer *lexer, s_errcont *errcont, s_ast *res)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    s_ast *redir = NULL;
    while (start_redir(tok)) { // TODO: HEREDOC
        s_ast **target;
        if (redir)
            target = &redir->data.ast_redirection.action;
        else
            target = &res->data.ast_block.redir;
        parse_redirection(target, lexer, errcont);
        redir = *target;
        tok = lexer_peek(lexer, errcont);
    }
    return res;
}

static s_ast *redirection_loop(s_lexer *lexer, s_ast *cmd, s_errcont *errcont)
{
    s_ast *res = xcalloc(sizeof(s_ast), 1);
    res->type = SHNODE_BLOCK;
    res->data.ast_block = ABLOCK(NULL, NULL, cmd);
    res = redirection_loop_sec(lexer, errcont, res);
    if (cmd->type == SHNODE_FUNCTION) {
        res->data.ast_block.cmd = cmd->data.ast_function.value;
        cmd->data.ast_function.value = res;
        return cmd;
    }
    return res;
}

static bool chose_shell_func(s_lexer *lexer, s_errcont *errcont, s_ast **res)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    bool shell =
        is_first_keyword(tok) || tok_is(tok, TOK_LBRACE) || tok_is(tok, TOK_LPAR);
    if (shell)
        parse_shell_command(res, lexer, errcont);
    else if (tok_is(tok, TOK_FUNC)) { // discard tokken 'function' and create token NAME
                                      // to match latter use
        tok_free(lexer_pop(lexer, errcont), true);
        parse_funcdec(res, lexer, errcont);
    } else
        return false;
    return true;
}

void parse_command(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    // TODO: change chose_shell_func api
    if (!chose_shell_func(lexer, errcont, res)) {
        s_token *word = lexer_peek(lexer, errcont);
        const s_token *tok = lexer_peek_at(lexer, word, errcont);
        bool is_par = tok_is(tok, TOK_LPAR);
        if (is_par)
            parse_funcdec(res, lexer, errcont);
        else {
            parse_simple_command(res, lexer, errcont);
            return;
        }
    }
    // TODO: remove obvious garbage
    *res = redirection_loop(lexer, *res, errcont);
}

static void switch_first_keyword(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_IF))
        parse_rule_if(res, lexer, errcont);
    else if (tok_is(tok, TOK_FOR))
        parse_rule_for(res, lexer, errcont);
    else if (tok_is(tok, TOK_WHILE))
        parse_rule_while(res, lexer, errcont);
    else if (tok_is(tok, TOK_UNTIL))
        parse_rule_until(res, lexer, errcont);
    else if (tok_is(tok, TOK_CASE))
        parse_rule_case(res, lexer, errcont);
    else
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s", TOKT_STR(tok));
}

static void parse_shell_command_sub(s_ast **res, s_lexer *lexer, s_errcont *errcont,
                                    bool par)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (!par && tok_is(tok, TOK_LPAR)) {
        *res = xcalloc(sizeof(s_ast), 1);
        (*res)->type = SHNODE_SUBSHELL;
        parse_shell_command_sub(&(*res)->data.ast_subshell.action, lexer, errcont, true);
    } else if (tok_is(tok, TOK_LBRACE) || par) {
        tok_free(lexer_pop(lexer, errcont), true);
        parse_compound_list(res, lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        if (!((tok_is(tok, TOK_RBRACE) && !par) || (tok_is(tok, TOK_RPAR) && par)))
            PARSER_ERROR(&tok->lineinfo, errcont,
                         "unexpected token %s, expected '}' or ')'", TOKT_STR(tok));
        tok_free(lexer_pop(lexer, errcont), true);
    } else
        switch_first_keyword(res, lexer, errcont);
}

void parse_shell_command(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    parse_shell_command_sub(res, lexer, errcont, false);
}
