#include "shlex/lexer.h"
#include "utils/alloc.h"
#include "utils/macros.h"
#include "shexec/clean_exit.h"
#include "shwlex/wlexer.h"
#include "utils/attr.h"

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

#define WLEXER_FORK(Wlexer, Mode)               \
    (struct wlexer) {                           \
        .cs = (Wlexer)->cs,                     \
        .mode = (Mode),                         \
    }

static void wtoken_push(struct token *token, struct wtoken *wtok) {
    for (char *cur = wtok->ch; *cur; cur++) {
        assert(cur < wtok->ch + 4);
        evect_push(&token->str, *cur);
    }
}

enum word_breaker_op {
    WBREAKER_FALLTHROUGH = 0,
    WBREAKER_CONTINUE = 1,
    WBREAKER_RETURN = 2,
    WBREAKER_PUSH = 4,
    WBREAKER_CANCEL = WBREAKER_RETURN | WBREAKER_PUSH,
};

enum word_breaker_op word_breaker_space(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    if (!isblank(wtoken->ch[0]))
        return WBREAKER_FALLTHROUGH;

    if (tok_size(token) != 0)
        return WBREAKER_CANCEL;

    // then wtoken isn't pushed by default
    return WBREAKER_CONTINUE;
}

enum word_breaker_op word_breaker_operator(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    const struct operator *operator = find_simple_operator(wtoken->ch[0]);
    if (operator == NULL)
        return WBREAKER_FALLTHROUGH;

    if (tok_size(token) != 0)
        return WBREAKER_CANCEL;

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
    return WBREAKER_RETURN;
}


enum word_breaker_op word_breaker_newline(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    int ch = wtoken->ch[0];
    if (ch != '\n')
        return WBREAKER_FALLTHROUGH;

    if (tok_size(token) != 0)
        return WBREAKER_CANCEL;

    token->type = TOK_NEWLINE;
    evect_push(&token->str, ch);
    return WBREAKER_RETURN;
}


enum word_breaker_op word_breaker_comment(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    if (wtoken->ch[0] != '#')
        return WBREAKER_FALLTHROUGH;

    if (tok_size(token) != 0)
        return WBREAKER_CANCEL;

    assert(!wlexer_has_lookahead(wlexer));
    do {
        // skip characters until the EOL
        int ch = cstream_peek(wlexer->cs);
        if (ch == EOF || ch == '\n')
            break;
        cstream_pop(wlexer->cs);
    } while (true);
    // continue parsing
    return WBREAKER_CONTINUE;
}


enum word_breaker_op word_breaker_regular(
    struct lexer *lexer __unused, struct wlexer *wlexer __unused,
    struct token *token, struct wtoken *wtoken) {
    // otherwise it's just a regular word
    wtoken_push(token, wtoken);
    return WBREAKER_CONTINUE;
}


// only break unquoted words
enum word_breaker_op word_breaker_quoting(
    struct lexer *lexer __unused, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken) {
    if (wlexer->mode == MODE_UNQUOTED)
        return WBREAKER_FALLTHROUGH;

    wtoken_push(token, wtoken);
    return WBREAKER_CONTINUE;
}


typedef enum word_breaker_op (*word_breaker)(
    struct lexer *lexer, struct wlexer *wlexer,
    struct token *token, struct wtoken *wtoken);

static word_breaker word_breakers[] = {
    word_breaker_quoting,
    word_breaker_space,
    word_breaker_comment,
    word_breaker_newline,
    word_breaker_operator,
    word_breaker_regular,
};

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
      for (size_t i = 0; i < ARR_SIZE(word_breakers); i++) {
          enum word_breaker_op op = word_breakers[i](lexer, wlexer, token, &wtok);
          if (op == WBREAKER_FALLTHROUGH)
              continue;
          if (op & WBREAKER_PUSH)
              wlexer_push(&wtok, wlexer);
          if (op & WBREAKER_RETURN)
              return 0;
          if (op & WBREAKER_CONTINUE)
              break;
      }
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
    {
        bool escape = false;
        wtoken_push(token, &wtok);
        do {
            memset(&wtok, 0, sizeof(wtok));
            wlexer_pop(&wtok, wlexer);
            if (wtok.type == WTOK_EOF)
                errx(1, "unexpected EOF in ` section\n");
            wtoken_push(token, &wtok);

            if (wtok.type == WTOK_ESCAPE)
                escape = true;
            else if (wtok.type == WTOK_BTICK) {
                if (!escape)
                    break;
                escape = false;
            }
        } while (true);
        break;
    }
    case WTOK_ESCAPE:
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
      break;
    case WTOK_SUBSH_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SUBSHELL), lexer);
      break;
    case WTOK_SUBSH_CLOSE:
      wtoken_push(token, &wtok);
      if (wlexer->mode == MODE_SUBSHELL)
          return 0;
      break;
    case WTOK_ARITH_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH), lexer);
      break;
    case WTOK_ARITH_CLOSE:
      wtoken_push(token, &wtok);
      if (wlexer->mode == MODE_ARITH)
          return 0;
      break;
    case WTOK_EXP_OPEN:
      wtoken_push(token, &wtok);
      lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXPANSION), lexer);
      break;
    case WTOK_EXP_CLOSE:
      wtoken_push(token, &wtok);
      if (wlexer->mode == MODE_EXPANSION)
          return 0;
      break;

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
