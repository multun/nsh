#pragma once

#include <stdbool.h>
#include <string.h>

#include "io/cstream.h"
#include "utils/attr.h"

struct wtoken
{
    char ch[4];

    enum wtoken_type
    {
        // used as a placeholder
        WTOK_UNKNOWN,

        // various single chars
        WTOK_SQUOTE,
        WTOK_DQUOTE,
        WTOK_BTICK,
        WTOK_ESCAPE,
        WTOK_EOF,

        WTOK_EXP_SUBSH_OPEN,
        WTOK_EXP_SUBSH_CLOSE, // single char
        WTOK_SUBSH_OPEN, // single char
        WTOK_SUBSH_CLOSE, // single char
        WTOK_ARITH_OPEN,
        WTOK_ARITH_CLOSE,
        WTOK_ARITH_GROUP_OPEN,
        WTOK_ARITH_GROUP_CLOSE,
        WTOK_EXP_OPEN,
        WTOK_EXP_CLOSE, // single char
        WTOK_REGULAR, // single char
    } type;
};

struct wlexer
{
    struct cstream *cs;
    enum wlexer_mode
    {
        MODE_UNQUOTED = 0,
        MODE_SINGLE_QUOTED,
        MODE_DOUBLE_QUOTED,
        MODE_EXP_SUBSHELL, // $()
        MODE_SUBSHELL, // ()
        MODE_ARITH,// $(( ))
        MODE_ARITH_GROUP, // $(( .. () .. ))
        MODE_EXPANSION, // ${}
    } mode;

    // lookahead buffer
    struct wtoken lookahead;
};

static inline void wlexer_set_errcont(struct wlexer *wlex, struct errcont *errcont)
{
    cstream_set_errcont(wlex->cs, errcont);
}

static inline struct lineinfo *wlexer_line_info(struct wlexer *wlexer)
{
    return &wlexer->cs->line_info;
}

static inline bool wlexer_in_arith(struct wlexer *wlexer)
{
    return wlexer->mode == MODE_ARITH || wlexer->mode == MODE_ARITH_GROUP;
}

static inline bool wlexer_has_lookahead(const struct wlexer *wlex)
{
    return wlex->lookahead.type != WTOK_UNKNOWN;
}

static inline void wlexer_clear_lookahead(struct wlexer *wlex)
{
    assert(wlexer_has_lookahead(wlex));
    wlex->lookahead.type = WTOK_UNKNOWN;
}

static inline void wlexer_init(struct wlexer *lexer, struct cstream *cs)
{
    lexer->cs = cs;
    lexer->mode = MODE_UNQUOTED;
    memset(&lexer->lookahead, 0, sizeof(lexer->lookahead));
}


#define WLEXER_FORK(Wlexer, Mode)                                                        \
    (struct wlexer)                                                                      \
    {                                                                                    \
        .cs = (Wlexer)->cs, .mode = (Mode),                                              \
    }

enum wlexer_op
{
    LEXER_OP_FALLTHROUGH = 0,
    LEXER_OP_CONTINUE = 1,
    LEXER_OP_RETURN = 2,
    LEXER_OP_PUSH = 4,
    LEXER_OP_CANCEL = LEXER_OP_RETURN | LEXER_OP_PUSH,
};


void wlexer_peek(struct wtoken *res, struct wlexer *lex);
enum wtoken_type wlexer_peek_type(struct wlexer *lex);
void wlexer_discard(struct wlexer *lex);
void wlexer_pop(struct wtoken *res, struct wlexer *lex);
void wlexer_push(const struct wtoken *res, struct wlexer *lex);

struct wlexer_btick_state {
    bool ran;
    bool escape;
};

#define WLEXER_BTICK_INIT { .ran = false, .escape = false }

static inline bool wlexer_btick_cond(struct wlexer_btick_state *state, struct wtoken *wtok)
{
    if (!state->ran)
        return true;

    if (state->escape)
        state->escape = false;
    else if (wtok->type == WTOK_ESCAPE)
        state->escape = true;
    else if (wtok->type == WTOK_BTICK)
        return false;
    return true;
}


static inline bool wlexer_btick_escaped(struct wlexer_btick_state *state)
{
    return state->escape;
}

#define WLEXER_BTICK_FOR(State, WTok)                                                    \
    for (; wlexer_btick_cond((State), (WTok)); (State)->ran = true)
