#include "shexec/clean_exit.h"
#include "shlex/lexer.h"
#include "shlex/variable.h"
#include "shwlex/wlexer.h"
#include "utils/alloc.h"
#include "utils/attr.h"
#include "utils/macros.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

__noreturn static void lexer_err(struct lexer *lexer, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsherror(lexer_line_info(lexer), lexer->errcont, &g_lexer_error, fmt, ap);

    va_end(ap);
}

static int lexer_lex_untyped(struct token *token, struct wlexer *wlexer,
                             struct lexer *lexer);

struct lexer *lexer_create(struct cstream *stream)
{
    struct lexer *res = xmalloc(sizeof(*res));
    wlexer_init(&res->wlexer, stream);
    res->head = NULL;
    return res;
}

void lexer_free(struct lexer *lexer)
{
    while (lexer->head) {
        struct token *tok = lexer->head;
        lexer->head = tok->next;
        tok_free(tok, true);
    }
    free(lexer);
}

static enum wlexer_op sublexer_eof(struct lexer *lexer, struct wlexer *wlexer,
                                  struct token *token, struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        lexer_err(lexer, "EOF while expecting quote");
    if (tok_size(token) == 0)
        token->type = TOK_EOF;
    return LEXER_OP_RETURN;
}

static enum wlexer_op sublexer_squote(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_dquote(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}


static enum wlexer_op sublexer_btick(struct lexer *lexer __unused, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtok)
{
    wtoken_push(token, wtok);
    struct wlexer_btick_state btick_state = WLEXER_BTICK_INIT;
    WLEXER_BTICK_FOR(&btick_state, wtok) {
        memset(wtok, 0, sizeof(*wtok));
        wlexer_pop(wtok, wlexer);
        if (wtok->type == WTOK_EOF)
            lexer_err(lexer, "unexpected EOF in ` section");
        evect_push(&token->str, wtok->ch[0]);
    }
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_escape(struct lexer *lexer __unused, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken __unused)
{
    // clearing characters isn't safe if
    // the wlexer has some cached tokens
    assert(!wlexer_has_lookahead(wlexer));
    int ch = cstream_pop(wlexer->cs);
    if (ch == EOF)
        lexer_err(lexer, "unexpected EOF in escape");

    // don't push carriage returns
    if (ch != '\n') {
        evect_push(&token->str, '\\');
        evect_push(&token->str, ch);
    }
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_exp_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                                             struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXP_SUBSHELL), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_EXP_SUBSH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_exp_subsh_close(struct lexer *lexer __unused,
                                               struct wlexer *wlexer,
                                               struct token *token __unused,
                                               struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_EXP_SUBSHELL);
    return LEXER_OP_RETURN;
}

static enum wlexer_op sublexer_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                                         struct token *token __unused, struct wtoken *wtoken)
{
    /* (echo test) */
    /* ^           */
    if (wlexer->mode == MODE_UNQUOTED)
    {
        /* oops(echo test) */
        /*     ^           */
        if (tok_size(token) != 0)
            /* break the first token appart */
            return LEXER_OP_CANCEL;
        wtoken_push(token, wtoken);
        token->type = TOK_LPAR;
        return LEXER_OP_RETURN;
    }

    /* $( (test)) */
    /*    ^       */
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SUBSHELL), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_SUBSH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_subsh_close(struct lexer *lexer __unused,
                                           struct wlexer *wlexer,
                                           struct token *token __unused,
                                           struct wtoken *wtoken __unused)
{
    /* (echo test) */
    /*           ^ */
    if (wlexer->mode == MODE_UNQUOTED)
    {
        /* oops(echo test) */
        /*               ^ */
        if (tok_size(token) != 0)
            return LEXER_OP_CANCEL;

        wtoken_push(token, wtoken);
        token->type = TOK_RPAR;
        return LEXER_OP_RETURN;
    }

    assert(wlexer->mode == MODE_SUBSHELL || wlexer->mode == MODE_UNQUOTED);
    return LEXER_OP_RETURN;
}

static enum wlexer_op sublexer_arith_open(struct lexer *lexer, struct wlexer *wlexer,
                                         struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH), lexer);
    struct wtoken end_wtoken = {
        .ch = {')', ')'},
        .type = WTOK_ARITH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_arith_close(struct lexer *lexer __unused,
                                           struct wlexer *wlexer,
                                           struct token *token __unused,
                                           struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH);
    return LEXER_OP_RETURN;
}

static enum wlexer_op sublexer_exp_open(struct lexer *lexer, struct wlexer *wlexer,
                                       struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXPANSION), lexer);
    struct wtoken end_wtoken = {
        .ch = {'}'},
        .type = WTOK_EXP_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_exp_close(struct lexer *lexer __unused,
                                         struct wlexer *wlexer,
                                         struct token *token __unused,
                                         struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_EXPANSION);
    return LEXER_OP_RETURN;
}

static enum wlexer_op sublexer_arith_group_open(struct lexer *lexer,
                                                struct wlexer *wlexer,
                                                struct token *token,
                                                struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH_GROUP), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_ARITH_GROUP_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op sublexer_arith_group_close(struct lexer *lexer __unused,
                                                 struct wlexer *wlexer,
                                                 struct token *token __unused,
                                                 struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH_GROUP);
    return LEXER_OP_RETURN;
}

static sublexer sublexers[] = {
    [WTOK_EOF] = sublexer_eof,
    [WTOK_REGULAR] = sublexer_regular,
    [WTOK_SQUOTE] = sublexer_squote,
    [WTOK_DQUOTE] = sublexer_dquote,
    [WTOK_BTICK] = sublexer_btick,
    [WTOK_ESCAPE] = sublexer_escape,
    [WTOK_EXP_SUBSH_OPEN] = sublexer_exp_subsh_open,
    [WTOK_EXP_SUBSH_CLOSE] = sublexer_exp_subsh_close,
    [WTOK_SUBSH_OPEN] = sublexer_subsh_open,
    [WTOK_SUBSH_CLOSE] = sublexer_subsh_close,
    [WTOK_ARITH_OPEN] = sublexer_arith_open,
    [WTOK_ARITH_CLOSE] = sublexer_arith_close,
    [WTOK_EXP_OPEN] = sublexer_exp_open,
    [WTOK_EXP_CLOSE] = sublexer_exp_close,
    [WTOK_ARITH_GROUP_OPEN] = sublexer_arith_group_open,
    [WTOK_ARITH_GROUP_CLOSE] = sublexer_arith_group_close,
};

static int lexer_lex_untyped(struct token *token, struct wlexer *wlexer,
                             struct lexer *lexer)
{
    token->type = TOK_WORD;
    while (true) {
        struct wtoken wtoken;
        memset(&wtoken, 0, sizeof(wtoken));

        wlexer_pop(&wtoken, wlexer);
        enum wlexer_op op = sublexers[wtoken.type](lexer, wlexer, token, &wtoken);
        // fallthrough doesn't make sense here
        assert(op != LEXER_OP_FALLTHROUGH);
        if (op & LEXER_OP_PUSH)
            wlexer_push(&wtoken, wlexer);
        if (op & LEXER_OP_RETURN)
            return 0;
        if (op & LEXER_OP_CONTINUE)
            continue;
    }
}

static bool is_only_digits(struct token *tok)
{
    for (size_t i = 0; i < tok_size(tok); i++)
        if (!isdigit(tok_buf(tok)[i]))
            return false;
    return true;
}

static const char *g_keywords[] = {
#define X(Type, Val) Val,
#include "shlex/keywords.defs"
#undef X
};

static int compare_keyword(const void *va, const void *vb)
{
    const char * const *a = va;
    const char * const *b = vb;
    return strcmp(*a, *b);
}

enum token_type keyword_search(const char *keyword)
{
    size_t keyword_count = ARR_SIZE(g_keywords);
    const char **search_res = bsearch(
        &keyword, g_keywords,
        keyword_count, sizeof(g_keywords[0]),
        compare_keyword);

    if (search_res == NULL)
        return TOK_UNDEF;

    /* TOK_KEYWORD_START_ is just before the keywords list */
    return (search_res - g_keywords) + 1 + TOK_KEYWORD_START_;
}

static void lexer_type_token(struct lexer *lexer, struct token *tok)
{
    if (tok->type != TOK_WORD)
        return;

    size_t tok_len = tok_size(tok) - 1;

    for (size_t i = 0; i < tok_len; i++)
        if (tok_buf(tok)[i] == '\0')
            lexer_err(lexer, "no input NUL bytes are allowed");

    if (is_only_digits(tok)) {
        struct wtoken next_tok;
        wlexer_peek(&next_tok, &lexer->wlexer);
        int ch = next_tok.ch[0];
        if (ch == '>' || ch == '<')
            tok->type = TOK_IO_NUMBER;
        return;
    }

    char *var_sep = strchr(tok_buf(tok), '=');
    if (var_sep != NULL) {
        size_t var_name_len = var_sep - tok_buf(tok);
        if (variable_name_check_string(tok_buf(tok), var_name_len) == 0)
            tok->type = TOK_ASSIGNMENT_WORD;
        /* in case something has an '=' char but no valid name,
        ** preserve the current TOK_WORD value of tok->type.
        */
        return;
    }

    /* for name keywords, the type will first be set as TOK_NAME,
    ** then by the correct type.
    */

    if (variable_name_check_string(tok_buf(tok), tok_len) == 0)
        tok->type = TOK_NAME;

    enum token_type search_type = keyword_search(tok_buf(tok));
    if (search_type != TOK_UNDEF)
        tok->type = search_type;
}

static void lexer_lex(struct token **tres, struct lexer *lexer, struct errcont *errcont)
{
    // the lexer and the IO stream both have their own global error contexts
    lexer->errcont = errcont;
    wlexer_set_errcont(&lexer->wlexer, errcont);

    struct token *res = *tres = tok_alloc(lexer);
    lexer_lex_untyped(res, &lexer->wlexer, lexer);
    tok_push(res, '\0');

    lexer_type_token(lexer, res);
}

char *lexer_lex_string(struct errcont *errcont, struct wlexer *wlexer)
{
    struct lexer lexer = {
        .wlexer = *wlexer,
        .errcont = errcont,
        .head = NULL,
    };

    lexer_lex(&lexer.head, &lexer, errcont);
    assert(lexer.head->next == NULL);
    char *buf = tok_buf(lexer.head);
    tok_free(lexer.head, false);
    return buf;
}

struct token *lexer_peek_at(struct lexer *lexer, struct token *tok, struct errcont *errcont)
{
    if (!tok->next)
        lexer_lex(&tok->next, lexer, errcont);
    return tok->next;
}

struct token *lexer_peek(struct lexer *lexer, struct errcont *errcont)
{
    if (!lexer->head)
        lexer_lex(&lexer->head, lexer, errcont);
    return lexer->head;
}

struct token *lexer_pop(struct lexer *lexer, struct errcont *errcont)
{
    if (!lexer->head)
        lexer_lex(&lexer->head, lexer, errcont);

    struct token *ret = lexer->head;
    lexer->head = ret->next;
    return ret;
}
