#pragma once

#include <stdbool.h>
#include <string.h>
#include "io/cstream.h"

struct wtoken {
  char ch[4];

  enum wtoken_type {
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
    WTOK_EXP_OPEN,
    WTOK_EXP_CLOSE, // single char
    WTOK_REGULAR, // single char
  } type;
};


struct wlexer {
  struct cstream *cs;
  enum wlexer_mode {
    MODE_UNQUOTED,
    MODE_SINGLE_QUOTED,
    MODE_DOUBLE_QUOTED,
    MODE_EXP_SUBSHELL,
    MODE_SUBSHELL,
    MODE_ARITH,
    MODE_EXPANSION,
  } mode;

  // lookahead buffer
  struct wtoken lookahead;
};

static inline bool wlexer_in_subshell(const struct wlexer *wlex) {
  return wlex->mode == MODE_EXP_SUBSHELL || wlex->mode == MODE_SUBSHELL;
}

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
