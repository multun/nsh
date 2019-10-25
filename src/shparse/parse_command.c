#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"

static int switch_first_keyword(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
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
        return 1;
    return 0;
}

static int parse_compound_command(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_LPAR)) {
        lexer_discard(lexer, errcont);
        struct shast_subshell *subshell = shast_subshell_attach(res, lexer);
        parse_compound_list(&subshell->action, lexer, errcont);
        parser_consume(lexer, TOK_RPAR, errcont);
        return 0;
    }

    if (tok_is(tok, TOK_LBRACE)) {
        lexer_discard(lexer, errcont);
        parse_compound_list(res, lexer, errcont);
        parser_consume(lexer, TOK_RBRACE, errcont);
        return 0;
    }

    return switch_first_keyword(res, lexer, errcont);
}

static int parse_base_command(struct lexer *lexer, struct errcont *errcont, struct shast **res)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (parse_compound_command(res, lexer, errcont) == 0)
        return 0;

    if (tok_is(tok, TOK_FUNC)) {
        lexer_discard(lexer, errcont);
        parse_funcdec(res, lexer, errcont);
        return 0;
    }

    struct token *word = lexer_peek(lexer, errcont);
    tok = lexer_peek_at(lexer, word, errcont);
    if (tok_is(tok, TOK_LPAR)) {
        parse_funcdec(res, lexer, errcont);
        return 0;
    }
    return 1;
}

void parse_command(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    if (parse_base_command(lexer, errcont, res) != 0) {
        // redirections are aready handled down there, as
        // they can be in the middle of the command as well
        parse_simple_command(res, lexer, errcont);
        return;
    }

    struct shast *old_root = *res;
    struct shast_block *block = shast_block_attach(res, lexer);
    block->command = old_root;

    while (parse_redirection(&block->redirs, lexer, errcont) == 0)
        continue;

    if (old_root->type == SHNODE_FUNCTION) {
        struct shast_function *func = (struct shast_function *)old_root;
        block->command = func->body;
        func->body = &block->base;
    }
}
