#include "shlex/lexer.h"
#include "utils/macros.h"

#include <assert.h>
#include <ctype.h>

#define LEX_OPS_MAP(TokName, Value) { Value, sizeof(Value) - 1, TokName },


static const struct operator
{
  const char *repr;
  size_t repr_size;
  enum token_type type;
} g_operators[] =
{
  LEX_OP_TOKS(LEX_OPS_MAP)
};


static const struct operator *find_operator(const char *buf, size_t size,
                                            char next_ch)
{
    // TODO: handle null bytes
    for (size_t i = 0; i < ARR_SIZE(g_operators); i++) {
        // skip operators shorter than the current buffer size
        if (g_operators[i].repr_size <= size)
            continue;

        if (strncmp(buf, g_operators[i].repr, size) == 0
            && g_operators[i].repr[size] == next_ch)
            return &g_operators[i];
    }

    return NULL;
}

static const struct operator *tok_find_operator(const struct token *token, int next_ch)
{
    if (next_ch == EOF)
        return NULL;
    return find_operator(tok_buf(token), tok_size(token), next_ch);
}

static const struct operator *find_simple_operator(int c) {
    return find_operator(NULL, 0, c);
}


static enum lexer_op word_breaker_space(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    if (!isblank(wtoken->ch[0]))
        return LEXER_OP_FALLTHROUGH;

    if (tok_size(token) != 0)
        return LEXER_OP_CANCEL;

    // then wtoken isn't pushed by default
    return LEXER_OP_CONTINUE;
}


static enum lexer_op word_breaker_operator(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    const struct operator *operator = find_simple_operator(wtoken->ch[0]);
    if (operator == NULL)
        return LEXER_OP_FALLTHROUGH;

    if (tok_size(token) != 0)
        return LEXER_OP_CANCEL;

    // we already found an operator, push it so we can use the token as a buffer
    evect_push(&token->str, wtoken->ch[0]);
    assert(!wlexer_has_lookahead(wlexer));
    do {
        int ch = cstream_peek(wlexer->cs);
        const struct operator *better_operator = tok_find_operator(token, ch);
        if (!better_operator)
            break;

        operator = better_operator;
        evect_push(&token->str, cstream_pop(wlexer->cs));
    } while (true);

    token->type = operator->type;
    return LEXER_OP_RETURN;
}


static enum lexer_op word_breaker_newline(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    int ch = wtoken->ch[0];
    if (ch != '\n')
        return LEXER_OP_FALLTHROUGH;

    if (tok_size(token) != 0)
        return LEXER_OP_CANCEL;

    token->type = TOK_NEWLINE;
    evect_push(&token->str, ch);
    return LEXER_OP_RETURN;
}


static enum lexer_op word_breaker_comment(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    if (wtoken->ch[0] != '#')
        return LEXER_OP_FALLTHROUGH;

    if (tok_size(token) != 0)
        return LEXER_OP_CANCEL;

    assert(!wlexer_has_lookahead(wlexer));
    do {
        // skip characters until the EOL
        int ch = cstream_peek(wlexer->cs);
        if (ch == EOF || ch == '\n')
            break;
        cstream_pop(wlexer->cs);
    } while (true);
    // continue parsing
    return LEXER_OP_CONTINUE;
}


static enum lexer_op word_breaker_regular(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    // otherwise it's just a regular word
    wtoken_push(token, wtoken);
    return LEXER_OP_CONTINUE;
}


// only break unquoted words
static enum lexer_op word_breaker_quoting(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    if (wlexer->mode == MODE_UNQUOTED)
        return LEXER_OP_FALLTHROUGH;

    wtoken_push(token, wtoken);
    return LEXER_OP_CONTINUE;
}


static sublexer word_breakers[] = {
    word_breaker_quoting,
    word_breaker_space,
    word_breaker_comment,
    word_breaker_newline,
    word_breaker_operator,
    word_breaker_regular,
};


enum lexer_op sublexer_regular(
    struct lexer *lexer, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    for (size_t i = 0; i < ARR_SIZE(word_breakers); i++) {
        enum lexer_op op = word_breakers[i](lexer, wlexer, token, wtoken);
        if (op == LEXER_OP_FALLTHROUGH)
            continue;

        return op;
    }
    assert(!"no sublexer matched, which makes no sense");
}
