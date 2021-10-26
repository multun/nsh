#pragma once

#include <stdbool.h>
#include <string.h>

#include <nsh_io/cstream.h>
#include <nsh_utils/attr.h>

enum wtoken_type
{
    // used as a placeholder
    WTOK_UNKNOWN = 0,
    WTOK_REGULAR, // a simple character
    WTOK_ESCAPE,  // <backslash>
    WTOK_EOF,     // models the end of file

    // various single chars
    WTOK_SQUOTE,  // '
    WTOK_DQUOTE,  // "
    WTOK_BTICK,   // `

    WTOK_VARIABLE,          // $
    WTOK_EXP_SUBSH_OPEN,    // $(
    WTOK_EXP_SUBSH_CLOSE,   // )
    WTOK_SUBSH_OPEN,        // (
    WTOK_SUBSH_CLOSE,       // )
    WTOK_ARITH_OPEN,        // $((
    WTOK_ARITH_CLOSE,       // ))
    WTOK_ARITH_GROUP_OPEN,  // (
    WTOK_ARITH_GROUP_CLOSE, // )
    WTOK_EXP_OPEN,          // ${
    WTOK_EXP_CLOSE,         // }
};

struct wtoken
{
    char ch[4];
    enum wtoken_type type;
};

enum wlexer_mode
{
    MODE_UNQUOTED = 1,
    MODE_SINGLE_QUOTED = 2,
    MODE_DOUBLE_QUOTED = 4,
    MODE_EXP_SUBSHELL = 8, // $()
    MODE_SUBSHELL = 16, // ()
    MODE_ARITH = 32,// $(( ))
    MODE_ARITH_GROUP = 64, // $(( .. () .. ))
    MODE_EXPANSION = 128, // ${}
};

#define WLEXER_ARITH_MODES (MODE_ARITH | MODE_ARITH_GROUP)

struct wlexer
{
    struct cstream *cs;
    enum wlexer_mode mode;
    struct wtoken lookahead;
};

static inline void wlexer_set_catcher(struct wlexer *wlex, struct exception_catcher *catcher)
{
    cstream_set_catcher(wlex->cs, catcher);
}

static inline struct lineinfo *wlexer_line_info(struct wlexer *wlexer)
{
    return &wlexer->cs->line_info;
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

static inline void wlexer_reset(struct wlexer *lexer)
{
    lexer->mode = MODE_UNQUOTED;
    memset(&lexer->lookahead, 0, sizeof(lexer->lookahead));
}

static inline void wlexer_init(struct wlexer *lexer, struct cstream *cs)
{
    lexer->cs = cs;
    wlexer_reset(lexer);
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
    /* push back into the wlexer */
    LEXER_OP_PUSH = 4,
    /* push back and return */
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

const char *wtoken_type_to_string(enum wtoken_type);
