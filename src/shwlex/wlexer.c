#include <assert.h>
#include <string.h>
#include "shwlex/wlexer.h"


static int wlexer_lex(struct wtoken *res, struct wlexer *lex);

int wlexer_peek(struct wtoken *res, struct wlexer *lex) {
  int rc;
  if (!wlexer_has_lookahead(lex))
    if ( (rc = wlexer_lex(res, lex) != 0))
      return rc;

  *res = lex->lookahead;
  return 0;
}


int wlexer_pop(struct wtoken *res, struct wlexer *lex) {
  if (!wlexer_has_lookahead(lex))
    return wlexer_lex(res, lex);

  *res = lex->lookahead;
  wlexer_clear_lookahead(lex);
  return 0;
}

int wlexer_push(const struct wtoken *res, struct wlexer *lex) {
  assert(!wlexer_has_lookahead(lex));
  lex->lookahead = *res;
  return 0;
}


static bool is_significant(struct wlexer *lex, int ch) {
  switch (ch) {
  case '"':	return lex->mode != MODE_SINGLE_QUOTED;
  case '\'':	return lex->mode != MODE_DOUBLE_QUOTED;
  case '`':	return lex->mode != MODE_SINGLE_QUOTED;
  case '\\':	return lex->mode != MODE_SINGLE_QUOTED;
  case '$':	return lex->mode != MODE_SINGLE_QUOTED;
  case '}':	return lex->mode == MODE_EXPANSION;
  default:
    return false;
  }
}


static int wtok_single_type(int ch) {
  switch (ch) {
  case '"':	return WTOK_DQUOTE;
  case '\'':	return WTOK_SQUOTE;
  case '`':	return WTOK_BTICK;
  case '\\':	return WTOK_ESCAPE;
  case '}':	return WTOK_EXP_CLOSE;
  default:
    return WTOK_UNKNOWN;
  }
}


static int wlexer_lex_dollar(struct wtoken *res,
			     struct wlexer *lex) {

  int ch = cstream_peek(lex->cs);

  if (lex->mode == MODE_SINGLE_QUOTED)
    goto wtok_regular;

  if (ch == '{') {
    res->ch[1] = cstream_pop(lex->cs);
    res->type = WTOK_EXP_OPEN;
    return 0;
  }

  if (ch != '(')
    goto wtok_regular;

  res->ch[1] = cstream_pop(lex->cs);
  ch = cstream_peek(lex->cs);
  if (ch == '(') {
    res->ch[2] = cstream_pop(lex->cs);
    res->type = WTOK_ARITH_OPEN;
  }
  else
    res->type = WTOK_SUBSH_OPEN;

  return 0;

wtok_regular:
  res->type = WTOK_REGULAR;
  return 0;
}


static int wlexer_lex_closing_paren(struct wtoken *res,
				    struct wlexer *lex) {
  if (lex->mode == MODE_SINGLE_QUOTED) {
    res->type = WTOK_REGULAR;
    return 0;
  }

  if (lex->mode == MODE_SUBSHELL) {
    res->type = WTOK_SUBSH_CLOSE;
    return 0;
  }

  int ch = cstream_peek(lex->cs);
  if (ch == ')' && lex->mode == MODE_ARITH) {
    res->ch[1] = cstream_pop(lex->cs);
    res->type = WTOK_ARITH_CLOSE;
    return 0;
  }

  res->type = WTOK_SUBSH_CLOSE;
  return 0;
}


static int wlexer_lex(struct wtoken *res, struct wlexer *lex) {
  memset(res->ch, 0, sizeof(res->ch));

  int ch = cstream_pop(lex->cs);
  if (ch == EOF) {
    res->type = WTOK_EOF;
    return 0;
  }

  res->ch[0] = ch;

  if ( is_significant (lex, ch) &&
       ( res->type = wtok_single_type(ch) ) != WTOK_UNKNOWN)
    return 0;

  switch (ch) {
  case '$':
    return wlexer_lex_dollar(res, lex);
  case ')':
    return wlexer_lex_closing_paren(res, lex);
  default:
    res->type = WTOK_REGULAR;
    return 0;
  }
}
