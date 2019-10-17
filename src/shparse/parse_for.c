#include <stdbool.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/error.h"

static void for_word_loop(s_wordlist **res, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    s_wordlist *tail = NULL;
    while (!tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_NEWLINE)) {
        s_wordlist **target;
        // TODO: simplify insertion, no need to differentiate cases
        if (!*res)
            target = res;
        else
            target = &tail->next;
        parse_word(target, lexer, errcont);
        tail = *target;
        tok = lexer_peek(lexer, errcont);
    }
    tok_free(lexer_pop(lexer, errcont), true);
}

static void parse_in(s_wordlist **words, s_lexer *lexer, s_errcont *errcont)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, TOK_NEWLINE) || tok_is(tok, TOK_IN)) {
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
        if (tok_is(tok, TOK_IN)) {
            tok_free(lexer_pop(lexer, errcont), true);
            for_word_loop(words, lexer, errcont);
            tok = lexer_peek(lexer, errcont);
        }
    }
    if (tok_is(tok, TOK_SEMI))
        tok_free(lexer_pop(lexer, errcont), true);
}

static bool parse_collection(s_lexer *lexer, s_errcont *errcont, s_ast *res)
{
    const s_token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DO)) {
        if (!tok_is(tok, TOK_NEWLINE) && !tok_is(tok, TOK_SEMI) && !tok_is(tok, TOK_IN))
            PARSER_ERROR(&tok->lineinfo, errcont,
                         "unexpected token %s, expected 'do', ';' or '\\n'",
                         TOKT_STR(tok));
        parse_in(&res->data.ast_for.collection, lexer, errcont);
        parse_newlines(lexer, errcont);
        tok = lexer_peek(lexer, errcont);
    }
    return true;
}

void parse_rule_for(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
    tok_free(lexer_pop(lexer, errcont), true);
    *res = ast_create(SHNODE_FOR, lexer);
    parse_word(&(*res)->data.ast_for.var, lexer, errcont);

    if (!parse_collection(lexer, errcont, *res))
        return;

    const s_token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, TOK_DO))
        PARSER_ERROR(&tok->lineinfo, errcont, "unexpected token %s, expected 'do'",
                     TOKT_STR(tok));
    parse_do_group(&(*res)->data.ast_for.actions, lexer, errcont);
}
