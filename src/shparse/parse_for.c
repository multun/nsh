#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/error.h"

static void for_word_loop(struct wordlist *target, struct lexer *lexer, struct errcont *errcont)
{
    while (true) {
        const struct token *tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_NEWLINE))
            break;
        wordlist_push(target, parse_word(lexer, errcont));
    }
    lexer_discard(lexer, errcont);
}

static void parse_in(struct wordlist *words, struct lexer *lexer, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN)) {
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_IN)) {
            lexer_discard(lexer, errcont);
            for_word_loop(words, lexer, errcont);
            tok = lexer_peek(lexer, errcont);
        }
    }
    if (tok_is(tok, TOK_SEMI))
        lexer_discard(lexer, errcont);
}

static bool parse_collection(struct lexer *lexer, struct errcont *errcont, struct shast_for *for_node)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DO)) {
        if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_IN))
            parser_err(&tok->lineinfo, errcont,
                       "unexpected token %s, expected 'do', ';' or '\\n'",
                       TOKT_STR(tok));
        parse_in(&for_node->collection, lexer, errcont);
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
    return true;
}

void parse_rule_for(struct shast **res, struct lexer *lexer, struct errcont *errcont)
{
    // TODO: safer discard
    lexer_discard(lexer, errcont);
    struct shast_for *for_node = shast_for_attach(res, lexer);
    for_node->var = parse_word(lexer, errcont);

    if (!parse_collection(lexer, errcont, for_node))
        return;

    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DO))
        parser_err(&tok->lineinfo, errcont, "unexpected token %s, expected 'do'",
                   TOKT_STR(tok));
    parse_do_group(&for_node->body, lexer, errcont);
}
