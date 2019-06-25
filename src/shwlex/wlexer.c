#include "shwlex/wlexer.h"

#include <assert.h>
#include <string.h>

static void wlexer_lex(struct wtoken *res, struct wlexer *lex);

enum wtoken_type wlexer_peek_type(struct wlexer *lex) {
    if (!wlexer_has_lookahead(lex))
        wlexer_lex(&lex->lookahead, lex);

    return lex->lookahead.type;
}

void wlexer_peek(struct wtoken *res, struct wlexer *lex)
{
    if (!wlexer_has_lookahead(lex))
        wlexer_lex(&lex->lookahead, lex);

    *res = lex->lookahead;
}

void wlexer_discard(struct wlexer *lex)
{
    if (!wlexer_has_lookahead(lex)) {
        struct wtoken res;
        wlexer_lex(&res, lex);
        return;
    }

    wlexer_clear_lookahead(lex);
    return;
}

void wlexer_pop(struct wtoken *res, struct wlexer *lex)
{
    if (!wlexer_has_lookahead(lex)) {
        wlexer_lex(res, lex);
        return;
    }

    *res = lex->lookahead;
    wlexer_clear_lookahead(lex);
    return;
}

void wlexer_push(const struct wtoken *res, struct wlexer *lex)
{
    assert(!wlexer_has_lookahead(lex));
    lex->lookahead = *res;
    return;
}

static bool is_significant(struct wlexer *lex, int ch)
{
    switch (ch) {
    case '(':
        return wlexer_in_subshell(lex);
    case '"':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '\'':
        return lex->mode != MODE_DOUBLE_QUOTED;
    case '`':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '\\':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '$':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '}':
        return lex->mode == MODE_EXPANSION;
    default:
        return false;
    }
}

static int wtok_single_type(int ch)
{
    switch (ch) {
    case '(':
        return WTOK_SUBSH_OPEN;
    case '"':
        return WTOK_DQUOTE;
    case '\'':
        return WTOK_SQUOTE;
    case '`':
        return WTOK_BTICK;
    case '\\':
        return WTOK_ESCAPE;
    case '}':
        return WTOK_EXP_CLOSE;
    default:
        return WTOK_UNKNOWN;
    }
}

static void wlexer_lex_dollar(struct wtoken *res, struct wlexer *lex)
{
    int ch = cstream_peek(lex->cs);

    if (lex->mode == MODE_SINGLE_QUOTED)
        goto wtok_regular;

    if (ch == '{') {
        res->ch[1] = cstream_pop(lex->cs);
        res->type = WTOK_EXP_OPEN;
        return;
    }

    if (ch != '(')
        goto wtok_regular;

    res->ch[1] = cstream_pop(lex->cs);
    ch = cstream_peek(lex->cs);
    if (ch == '(') {
        res->ch[2] = cstream_pop(lex->cs);
        res->type = WTOK_ARITH_OPEN;
    } else
        res->type = WTOK_EXP_SUBSH_OPEN;

    return;

wtok_regular:
    res->type = WTOK_REGULAR;
    return;
}

static void wlexer_lex_closing_paren(struct wtoken *res, struct wlexer *lex)
{
    // the type of a right paren is context dependant.
    // if we're not in a subshell or we're in arith mode
    // and there's only a single paren, it's just a regular
    // token.
    if (lex->mode == MODE_EXP_SUBSHELL)
        res->type = WTOK_EXP_SUBSH_CLOSE;
    else if (lex->mode == MODE_SUBSHELL)
        res->type = WTOK_SUBSH_CLOSE;
    else if (lex->mode == MODE_ARITH) {
        int ch = cstream_peek(lex->cs);
        if (ch != ')')
            goto regular;

        res->ch[1] = cstream_pop(lex->cs);
        res->type = WTOK_ARITH_CLOSE;
        return;
    } else
        goto regular;

    return;

regular:
    res->type = WTOK_REGULAR;
    return;
}

static void wlexer_lex(struct wtoken *res, struct wlexer *lex)
{
    memset(res->ch, 0, sizeof(res->ch));

    int ch = cstream_pop(lex->cs);
    if (ch == EOF) {
        res->type = WTOK_EOF;
        return;
    }

    res->ch[0] = ch;

    if (is_significant(lex, ch) && (res->type = wtok_single_type(ch)) != WTOK_UNKNOWN)
        return;

    switch (ch) {
    case '$':
        wlexer_lex_dollar(res, lex);
        break;
    case ')':
        wlexer_lex_closing_paren(res, lex);
        break;
    default:
        res->type = WTOK_REGULAR;
        return;
    }
}
