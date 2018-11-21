#include "shlex/lexer.h"
#include "utils/alloc.h"
#include "utils/macros.h"
#include "shexec/clean_exit.h"
#include "shwlex/wlexer.h"

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>


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


static bool starts_operator(char c)
{
  for (size_t i = 0; i < ARR_SIZE(g_operators); i++)
    if (c ==  g_operators[i].repr[0])
      return true;
  return false;
}


static bool is_breaking(char c)
{
  return isblank(c) || c == '\n' || c == '#' || starts_operator(c);
}


s_lexer *lexer_create(s_cstream *stream)
{
  s_lexer *res = xmalloc(sizeof(*res));
  wlexer_init(&res->wlexer, stream);
  res->head = NULL;
  return res;
}


void lexer_free(s_lexer *lexer)
{
  while (lexer->head)
  {
    s_token *tok = lexer->head;
    lexer->head = tok->next;
    tok_free(tok, true);
  }
  free(lexer);
}


static bool is_only_digits(s_token *tok)
{
  for (size_t i = 0; i < TOK_SIZE(tok); i++)
    if (!isdigit(TOK_STR(tok)[i]))
      return false;
  return true;
}

#define WLEXER_FORK(Wlexer, Mode)		\
  (struct wlexer) {				\
    .cs = (Wlexer)->cs,				\
    .mode = (Mode),				\
  }

static void wtoken_push(struct token *token, struct wtoken *wtok) {
  for (char *cur = wtok->ch; *cur; cur++) {
    assert(cur < wtok->ch + 4);
    evect_push(&token->str, *cur);
  }
}

int lexer_lex_untyped(struct token *token,
		      struct wlexer *wlexer,
		      struct lexer *lexer) {
  token->type = TOK_WORD;
  while (true) {
    struct wtoken wtok;
    memset(&wtok, 0, sizeof (wtok));

    wlexer_pop(&wtok, wlexer);
    switch (wtok.type) {
    case WTOK_EOF:
      if (wlexer->mode != MODE_UNQUOTED)
	errx(1, "EOF in not unquoted\n");
      if (token->str.size == 0)
	token->type = TOK_EOF;
      return 0;
    case WTOK_REGULAR:
      if (wlexer->mode != MODE_UNQUOTED) {
	wtoken_push(token, &wtok);
	break;
      }

      /* if the next char would break and there's already
       * something in the buffer, push it back and stop
       */

      if (is_breaking(wtok.ch[0]) &&
	  token->str.size != 0) {
	wlexer_push(&wtok, wlexer);
        return 0;
      }

      if (wtok.ch[0] == '#') {
	// clearing characters isn't safe if
	// the wlexer has some cached tokens
	assert(!wlexer_has_lookahead(wlexer));
	int ch;
	do {
	  // skip characters until the EOL
	  ch = cstream_peek(wlexer->cs);
	  if (ch == EOF || ch == '\n')
	    break;
	  cstream_pop(wlexer->cs);
	} while (true);
	// continue parsing
	break;
      }

      if (wtok.ch[0] == '\n') {
	token->type = TOK_NEWLINE;
	wtoken_push(token, &wtok);
	return 0;
      }

      // skip spaces
      if (isblank(wtok.ch[0]) && token->str.size == 0)
	break;

      // otherwise it's just a regular word
      wtoken_push(token, &wtok);
      break;
    case WTOK_SQUOTE:
      wtoken_push(token, &wtok);
      if (wlexer->mode == MODE_SINGLE_QUOTED)
	return 0;

      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED), lexer);
      break;
    case WTOK_DQUOTE:
      wtoken_push(token, &wtok);
      if (wlexer->mode == MODE_DOUBLE_QUOTED)
	return 0;

      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED), lexer);
      break;
    case WTOK_BTICK:
      wtoken_push(token, &wtok);
      do {
	memset(&wtok, 0, sizeof(wtok));
	wlexer_pop(&wtok, wlexer);
	if (wtok.type == WTOK_EOF)
	  errx(1, "unexpected EOF in ` section\n");
	wtoken_push(token, &wtok);
      } while (wtok.type != WTOK_BTICK);
      break;
    case WTOK_ESCAPE:
      // clearing characters isn't safe if
      // the wlexer has some cached tokens
      assert(!wlexer_has_lookahead(wlexer));
      int ch = cstream_pop(wlexer->cs);
      if (ch == EOF)
	errx(1, "unexpected EOF in escape\n");

      // don't push carriage returns
      if (ch != '\n')
	evect_push(&token->str, ch);
      break;
    case WTOK_SUBSH_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SUBSHELL), lexer);
      break;
    case WTOK_SUBSH_CLOSE:
      wtoken_push(token, &wtok);
      assert(wlexer->mode == MODE_SUBSHELL);
      return 0;
    case WTOK_ARITH_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH), lexer);
      break;
    case WTOK_ARITH_CLOSE:
      wtoken_push(token, &wtok);
      assert(wlexer->mode == MODE_ARITH);
      return 0;
    case WTOK_EXP_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXPANSION), lexer);
      break;
    case WTOK_EXP_CLOSE:
      wtoken_push(token, &wtok);
      assert(wlexer->mode == MODE_EXPANSION);
      return 0;

    case WTOK_UNKNOWN:
      errx(1, "lol?");
    }
  }
  return 0;
}


static void lexer_lex(s_token **tres, s_lexer *lexer, s_errcont *errcont)
{
  s_token *res = *tres = tok_alloc(lexer);

  lexer_lex_untyped(res, &lexer->wlexer, lexer);

  if (res->type != TOK_WORD)
    return;

  for (size_t i = 0; i < TOK_SIZE(res); i++)
    clean_assert(errcont, res->str.data[i], "no input NUL bytes are allowed");

  if (is_only_digits(res)) {
    struct wtoken next_tok;
    wlexer_peek(&next_tok, &lexer->wlexer);
    int ch = next_tok.ch[0];
    if (ch == '>' || ch == '<')
      res->type = TOK_IO_NUMBER;
    return;
  }

  TOK_PUSH(res, '\0');
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
