#include "shwlex/wlexer.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>


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

static bool is_dquoted_escape_special(int next_char)
{
    switch (next_char) {
    case '$':
    case '`':
    case '"':
    case '\\':
        return true;
    default:
        return false;
    }
}

static bool is_significant(struct wlexer *lex, int ch)
{
    switch (ch) {
    case '(':
        switch (lex->mode) {
        case MODE_EXP_SUBSHELL:
        case MODE_SUBSHELL:
        case MODE_ARITH:
        case MODE_ARITH_GROUP:
        case MODE_UNQUOTED:
            return true;
        default:
            return false;
        }
    case '"':
        switch (lex->mode) {
        case MODE_SINGLE_QUOTED:
        case MODE_ARITH:
        case MODE_ARITH_GROUP:
            return false;
        default:
            return true;
        }
    case '\'':
        return lex->mode != MODE_DOUBLE_QUOTED;
    case '`':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '\\':
        // TODO: MODE_EXPANSION specific handling
        switch (lex->mode) {
        case MODE_SINGLE_QUOTED:
            return false;
        case MODE_ARITH:
            if (cstream_peek(lex->cs) == '"')
                return false;
            /* fall through */
        case MODE_DOUBLE_QUOTED:
            return is_dquoted_escape_special(cstream_peek(lex->cs));
        default:
            return true;
        }
    case '$':
        return lex->mode != MODE_SINGLE_QUOTED;
    case '}':
        return lex->mode == MODE_EXPANSION;
    default:
        return false;
    }
}

static int wtok_single_type(struct wlexer *lex, int ch)
{
    switch (ch) {
    case '(':
        if (wlexer_in_arith(lex))
            return WTOK_ARITH_GROUP_OPEN;
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
    if (lex->mode == MODE_ARITH_GROUP)
        res->type = WTOK_ARITH_GROUP_CLOSE;
    else if (lex->mode == MODE_EXP_SUBSHELL)
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

    if (is_significant(lex, ch)) {
        res->type = wtok_single_type(lex, ch);
        if (res->type != WTOK_UNKNOWN)
            return;
    }

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


const char *wtoken_type_to_string(enum wtoken_type type)
{
    switch (type) {
    case WTOK_UNKNOWN:
        return "unknown";
    case WTOK_SQUOTE:
        return "simple quote";
    case WTOK_DQUOTE:
        return "double quote";
    case WTOK_BTICK:
        return "backtick";
    case WTOK_ESCAPE:
        return "escape";
    case WTOK_EOF:
        return "EOF";
    case WTOK_EXP_SUBSH_OPEN:
        return "expansion subshell open";
    case WTOK_EXP_SUBSH_CLOSE:
        return "expansion subshell close";
    case WTOK_SUBSH_OPEN:
        return "subshell open";
    case WTOK_SUBSH_CLOSE:
        return "subshell close";
    case WTOK_ARITH_OPEN:
        return "arithmetic open";
    case WTOK_ARITH_CLOSE:
        return "arithmetic clone";
    case WTOK_ARITH_GROUP_OPEN:
        return "arithmetic group open";
    case WTOK_ARITH_GROUP_CLOSE:
        return "arithmetic group close";
    case WTOK_EXP_OPEN:
        return "expansion open";
    case WTOK_EXP_CLOSE:
        return "expansion close";
    case WTOK_REGULAR:
        return "regular";
    default:
        abort();
    }
}
