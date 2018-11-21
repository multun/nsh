#pragma once

#include <stdbool.h>
#include <string.h>
#include "io/cstream.h"

struct wtoken {
  char ch[4];

  enum wtoken_type {
    // used as a placeholder
    WTOK_UNKNOWN,

    WTOK_SQUOTE,
    WTOK_DQUOTE,
    WTOK_BTICK,
    WTOK_ESCAPE,
    WTOK_EOF,
    // longer than a single char
    WTOK_SUBSH_OPEN,
    WTOK_SUBSH_CLOSE,
    WTOK_ARITH_OPEN,
    WTOK_ARITH_CLOSE,
    WTOK_EXP_OPEN,
    WTOK_EXP_CLOSE,
    WTOK_REGULAR,
  } type;
};


struct wlexer {
  struct cstream *cs;
  enum wlexer_mode {
    MODE_UNQUOTED,
    MODE_SINGLE_QUOTED,
    MODE_DOUBLE_QUOTED,
    MODE_SUBSHELL,
    MODE_ARITH,
    MODE_EXPANSION,
  } mode;

  // lookahead buffer
  struct wtoken lookahead;
};

static inline bool wlexer_has_lookahead(const struct wlexer *wlex) {
  return wlex->lookahead.type != WTOK_UNKNOWN;
}

static inline void wlexer_clear_lookahead(struct wlexer *wlex) {
  wlex->lookahead.type = WTOK_UNKNOWN;
}

static inline void wlexer_init(struct wlexer *lexer, struct cstream *cs) {
  memset(lexer, 0, sizeof(*lexer));
  lexer->cs = cs;
}

int wlexer_peek(struct wtoken *res, struct wlexer *lex);
int wlexer_pop(struct wtoken *res, struct wlexer *lex);
int wlexer_push(const struct wtoken *res, struct wlexer *lex);
