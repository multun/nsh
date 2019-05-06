#include "shlex/lexer.h"
#include "shexec/clean_exit.h"
#include "shwlex/wlexer.h"
#include "utils/alloc.h"
#include "utils/attr.h"
#include "utils/macros.h"

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

static int lexer_lex_untyped(struct token *token, struct wlexer *wlexer,
                             struct lexer *lexer);

s_lexer *lexer_create(s_cstream *stream)
{
    s_lexer *res = xmalloc(sizeof(*res));
    wlexer_init(&res->wlexer, stream);
    res->head = NULL;
    return res;
}

void lexer_free(s_lexer *lexer)
{
    while (lexer->head) {
        s_token *tok = lexer->head;
        lexer->head = tok->next;
        tok_free(tok, true);
    }
    free(lexer);
}

static enum lexer_op sublexer_eof(struct lexer *lexer __unused, struct wlexer *wlexer,
                                  struct token *token, struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        errx(1, "EOF in not unquoted\n");
    if (token->str.size == 0)
        token->type = TOK_EOF;
    return LEXER_OP_RETURN;
}

static enum lexer_op sublexer_squote(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_dquote(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_btick(struct lexer *lexer __unused, struct wlexer *wlexer,
                                    struct token *token, struct wtoken *wtok)
{
    bool escape = false;
    wtoken_push(token, wtok);
    do {
        memset(wtok, 0, sizeof(*wtok));
        wlexer_pop(wtok, wlexer);
        if (wtok->type == WTOK_EOF)
            errx(1, "unexpected EOF in ` section\n");
        wtoken_push(token, wtok);

        if (escape)
            escape = false;
        else if (wtok->type == WTOK_ESCAPE)
            escape = true;
        else if (wtok->type == WTOK_BTICK)
            break;
    } while (true);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_escape(struct lexer *lexer __unused, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken __unused)
{
    // clearing characters isn't safe if
    // the wlexer has some cached tokens
    assert(!wlexer_has_lookahead(wlexer));
    int ch = cstream_pop(wlexer->cs);
    if (ch == EOF)
        errx(1, "unexpected EOF in escape\n");

    // don't push carriage returns
    if (ch != '\n') {
        evect_push(&token->str, '\\');
        evect_push(&token->str, ch);
    }
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_exp_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                                             struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXP_SUBSHELL), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_exp_subsh_close(struct lexer *lexer __unused,
                                              struct wlexer *wlexer, struct token *token,
                                              struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    assert(wlexer->mode == MODE_EXP_SUBSHELL);
    return LEXER_OP_RETURN;
}

static enum lexer_op sublexer_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                                         struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SUBSHELL), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_subsh_close(struct lexer *lexer __unused,
                                          struct wlexer *wlexer, struct token *token,
                                          struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    assert(wlexer->mode == MODE_SUBSHELL);
    return LEXER_OP_RETURN;
}

static enum lexer_op sublexer_arith_open(struct lexer *lexer, struct wlexer *wlexer,
                                         struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_arith_close(struct lexer *lexer __unused,
                                          struct wlexer *wlexer, struct token *token,
                                          struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    assert(wlexer->mode == MODE_ARITH);
    return LEXER_OP_RETURN;
}

static enum lexer_op sublexer_exp_open(struct lexer *lexer, struct wlexer *wlexer,
                                       struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXPANSION), lexer);
    return LEXER_OP_CONTINUE;
}

static enum lexer_op sublexer_exp_close(struct lexer *lexer __unused,
                                        struct wlexer *wlexer, struct token *token,
                                        struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    assert(wlexer->mode == MODE_EXPANSION);
    return LEXER_OP_RETURN;
}

static sublexer sublexers[] = {
    [WTOK_EOF] = sublexer_eof,
    [WTOK_REGULAR] = sublexer_regular,
    [WTOK_SQUOTE] = sublexer_squote,
    [WTOK_DQUOTE] = sublexer_dquote,
    [WTOK_BTICK] = sublexer_btick,
    [WTOK_ESCAPE] = sublexer_escape,
    [WTOK_EXP_SUBSH_OPEN] = sublexer_exp_subsh_open,
    [WTOK_EXP_SUBSH_CLOSE] = sublexer_exp_subsh_close,
    [WTOK_SUBSH_OPEN] = sublexer_subsh_open,
    [WTOK_SUBSH_CLOSE] = sublexer_subsh_close,
    [WTOK_ARITH_OPEN] = sublexer_arith_open,
    [WTOK_ARITH_CLOSE] = sublexer_arith_close,
    [WTOK_EXP_OPEN] = sublexer_exp_open,
    [WTOK_EXP_CLOSE] = sublexer_exp_close,
};

static int lexer_lex_untyped(struct token *token, struct wlexer *wlexer,
                             struct lexer *lexer)
{
    token->type = TOK_WORD;
    while (true) {
        struct wtoken wtoken;
        memset(&wtoken, 0, sizeof(wtoken));

        wlexer_pop(&wtoken, wlexer);
        enum lexer_op op = sublexers[wtoken.type](lexer, wlexer, token, &wtoken);
        // fallthrough doesn't make sense here
        assert(op != LEXER_OP_FALLTHROUGH);
        if (op & LEXER_OP_PUSH)
            wlexer_push(&wtoken, wlexer);
        if (op & LEXER_OP_RETURN)
            return 0;
        if (op & LEXER_OP_CONTINUE)
            continue;
    }
}

static bool is_only_digits(s_token *tok)
{
    for (size_t i = 0; i < tok_size(tok); i++)
        if (!isdigit(tok_buf(tok)[i]))
            return false;
    return true;
}

static void lexer_lex(s_token **tres, s_lexer *lexer, s_errcont *errcont)
{
    s_token *res = *tres = tok_alloc(lexer);

    lexer_lex_untyped(res, &lexer->wlexer, lexer);

    if (res->type != TOK_WORD)
        goto typing_done;

    for (size_t i = 0; i < tok_size(res); i++)
        clean_assert(errcont, res->str.data[i], "no input NUL bytes are allowed");

    if (is_only_digits(res)) {
        struct wtoken next_tok;
        wlexer_peek(&next_tok, &lexer->wlexer);
        int ch = next_tok.ch[0];
        if (ch == '>' || ch == '<')
            res->type = TOK_IO_NUMBER;
        return;
    }

typing_done:
    tok_push(res, '\0');
    return;
}

s_token *lexer_peek_at(s_lexer *lexer, s_token *tok, s_errcont *errcont)
{
    if (!tok->next)
        lexer_lex(&tok->next, lexer, errcont);
    return tok->next;
}

s_token *lexer_peek(s_lexer *lexer, s_errcont *errcont)
{
    if (!lexer->head)
        lexer_lex(&lexer->head, lexer, errcont);
    return lexer->head;
}

s_token *lexer_pop(s_lexer *lexer, s_errcont *errcont)
{
    if (!lexer->head)
        lexer_lex(&lexer->head, lexer, errcont);

    s_token *ret = lexer->head;
    lexer->head = ret->next;
    return ret;
}
