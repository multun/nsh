#include <stdbool.h>

#include <nsh_parse/parse.h>
#include <nsh_lex/print.h>
#include <nsh_utils/exception.h>

static void for_word_loop(struct wordlist *target, struct lexer *lexer,
                          struct exception_catcher *catcher)
{
    while (true) {
        const struct token *tok = lexer_peek(lexer, catcher);
        if (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_NEWLINE))
            break;
        wordlist_push(target, parse_word(lexer, catcher));
    }
    lexer_discard(lexer, catcher);
}

static void parse_in(struct wordlist *words, struct lexer *lexer,
                     struct exception_catcher *catcher)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN)) {
        parse_newlines(lexer, catcher);
        tok = lexer_peek(lexer, catcher);
        if (tok_is(tok, TOK_IN)) {
            lexer_discard(lexer, catcher);
            for_word_loop(words, lexer, catcher);
            tok = lexer_peek(lexer, catcher);
        }
    }
    if (tok_is(tok, TOK_SEMI))
        lexer_discard(lexer, catcher);
}

static bool parse_collection(struct lexer *lexer, struct exception_catcher *catcher,
                             struct shast_for *for_node)
{
    const struct token *tok = lexer_peek(lexer, catcher);
    if (!tok_is(tok, TOK_DO)) {
        if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_IN))
            parser_err(&tok->lineinfo, catcher,
                       "unexpected token %s, expected 'do', ';' or '\\n'", TOKT_STR(tok));
        parse_in(&for_node->collection, lexer, catcher);
        parse_newlines(lexer, catcher);
        tok = lexer_peek(lexer, catcher);
    }
    return true;
}

void parse_rule_for(struct shast **res, struct lexer *lexer,
                    struct exception_catcher *catcher)
{
    // TODO: safer discard
    lexer_discard(lexer, catcher);
    struct shast_for *for_node = shast_for_attach(res, lexer);
    for_node->var = parse_word(lexer, catcher);

    if (!parse_collection(lexer, catcher, for_node))
        return;

    const struct token *tok = lexer_peek(lexer, catcher);
    if (!tok_is(tok, TOK_DO))
        parser_err(&tok->lineinfo, catcher, "unexpected token %s, expected 'do'",
                   TOKT_STR(tok));
    parse_do_group(&for_node->body, lexer, catcher);
}
