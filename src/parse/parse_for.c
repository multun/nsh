#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/error.h"

static void for_word_loop(struct wordlist *target, struct lexer *lexer, struct ex_scope *ex_scope)
{
    while (true) {
        const struct token *tok = lexer_peek(lexer, ex_scope);
        if (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_NEWLINE))
            break;
        wordlist_push(target, parse_word(lexer, ex_scope));
    }
    lexer_discard(lexer, ex_scope);
}

static void parse_in(struct wordlist *words, struct lexer *lexer, struct ex_scope *ex_scope)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN)) {
        parse_newlines(lexer, ex_scope);
        tok = lexer_peek(lexer, ex_scope);
        if (tok_is(tok, TOK_IN)) {
            lexer_discard(lexer, ex_scope);
            for_word_loop(words, lexer, ex_scope);
            tok = lexer_peek(lexer, ex_scope);
        }
    }
    if (tok_is(tok, TOK_SEMI))
        lexer_discard(lexer, ex_scope);
}

static bool parse_collection(struct lexer *lexer, struct ex_scope *ex_scope, struct shast_for *for_node)
{
    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (!tok_is(tok, TOK_DO)) {
        if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_IN))
            parser_err(&tok->lineinfo, ex_scope,
                       "unexpected token %s, expected 'do', ';' or '\\n'",
                       TOKT_STR(tok));
        parse_in(&for_node->collection, lexer, ex_scope);
        parse_newlines(lexer, ex_scope);
        tok = lexer_peek(lexer, ex_scope);
    }
    return true;
}

void parse_rule_for(struct shast **res, struct lexer *lexer, struct ex_scope *ex_scope)
{
    // TODO: safer discard
    lexer_discard(lexer, ex_scope);
    struct shast_for *for_node = shast_for_attach(res, lexer);
    for_node->var = parse_word(lexer, ex_scope);

    if (!parse_collection(lexer, ex_scope, for_node))
        return;

    const struct token *tok = lexer_peek(lexer, ex_scope);
    if (!tok_is(tok, TOK_DO))
        parser_err(&tok->lineinfo, ex_scope, "unexpected token %s, expected 'do'",
                   TOKT_STR(tok));
    parse_do_group(&for_node->body, lexer, ex_scope);
}
