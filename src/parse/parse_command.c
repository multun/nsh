#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/error.h>

static int switch_first_keyword(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (tok_is(tok, TOK_IF))
        parse_rule_if(res, lexer, ex_scope);
    else if (tok_is(tok, TOK_FOR))
        parse_rule_for(res, lexer, ex_scope);
    else if (tok_is(tok, TOK_WHILE))
        parse_rule_while(res, lexer, ex_scope);
    else if (tok_is(tok, TOK_UNTIL))
        parse_rule_until(res, lexer, ex_scope);
    else if (tok_is(tok, TOK_CASE))
        parse_rule_case(res, lexer, ex_scope);
    else
        return 1;
    return 0;
}

static int parse_compound_command(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (tok_is(tok, TOK_LPAR)) {
        lexer_discard(lexer, ex_scope);
        struct shast_subshell *subshell = shast_subshell_attach(res, lexer);
        parse_compound_list(&subshell->action, lexer, ex_scope);
        parser_consume(lexer, TOK_RPAR, ex_scope);
        return 0;
    }

    if (tok_is(tok, TOK_LBRACE)) {
        lexer_discard(lexer, ex_scope);
        parse_compound_list(res, lexer, ex_scope);
        parser_consume(lexer, TOK_RBRACE, ex_scope);
        return 0;
    }

    return switch_first_keyword(res, lexer, ex_scope);
}

static int parse_base_command(struct lexer *lexer, struct ex_scope *ex_scope, struct shast **res)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (parse_compound_command(res, lexer, ex_scope) == 0)
        return 0;

    if (tok_is(tok, TOK_FUNC)) {
        lexer_discard(lexer, ex_scope);
        parse_funcdec(res, lexer, ex_scope);
        return 0;
    }

    struct token *word = lexer_peek(lexer, ex_scope);
    tok = lexer_peek_at(lexer, word, ex_scope);
    if (tok_is(tok, TOK_LPAR)) {
        parse_funcdec(res, lexer, ex_scope);
        return 0;
    }
    return 1;
}

void parse_command(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    if (parse_base_command(lexer, ex_scope, res) != 0) {
        // redirections are aready handled down there, as
        // they can be in the middle of the command as well
        parse_simple_command(res, lexer, ex_scope);
        return;
    }

    struct shast *old_root = *res;
    struct shast_block *block = shast_block_attach(res, lexer);
    block->command = old_root;

    while (parse_redirection(&block->redirs, lexer, ex_scope) == 0)
        continue;

    if (old_root->type == SHNODE_FUNCTION) {
        struct shast_function *func = (struct shast_function *)old_root;
        block->command = func->body;
        func->body = &block->base;
        *res = &func->base;
    }
}

void parse_funcdec(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    struct shast_function *func = shast_function_attach(res, lexer);
    struct token *word = lexer_pop(lexer, ex_scope);
    hash_head_init(&func->hash, tok_buf(word));
    tok_free(word, false);
    parser_consume(lexer, TOK_LPAR, ex_scope);
    parser_consume(lexer, TOK_RPAR, ex_scope);
    parse_newlines(lexer, ex_scope);
    parse_compound_command(&func->body, lexer, ex_scope);
}
