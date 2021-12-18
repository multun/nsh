#include <nsh_lex/lexer.h>
#include <nsh_lex/variable.h>
#include <nsh_lex/wlexer.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/macros.h>
#include <nsh_utils/logging.h>

#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


static inline size_t lexer_queue_size(const struct lexer *lexer)
{
    return lexer->buffer_meta.size;
}

static inline struct token *lexer_queue_get(struct lexer *lex, size_t i)
{
    size_t buf_i = ring_buffer_getindex(&lex->buffer_meta, LEXER_LOOKAHEAD, i);
    return lex->next_tokens[buf_i];
}

static inline void lexer_queue_push(struct lexer *lex, struct token *tok)
{
    size_t buf_i = ring_buffer_push(&lex->buffer_meta, LEXER_LOOKAHEAD);
    lex->next_tokens[buf_i] = tok;
}

static inline struct token *lexer_queue_pop(struct lexer *lex)
{
    size_t buf_i = ring_buffer_pop(&lex->buffer_meta, LEXER_LOOKAHEAD);
    struct token *res = lex->next_tokens[buf_i];
    lex->next_tokens[buf_i] = NULL;
    return res;
}

nsh_err_t lexer_err(struct lexer *lexer, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    lineinfo_vwarn(&lexer->wlexer.cs->line_info, fmt, ap);

    va_end(ap);
    return NSH_LEXER_ERROR;
}

int lexer_lex_untyped(struct token *token, struct wlexer *wlexer, struct lexer *lexer)
{
    int rc;
    token->type = TOK_WORD;
    while (true) {
        struct wtoken wtoken;
        memset(&wtoken, 0, sizeof(wtoken));

        if ((rc = wlexer_pop(&wtoken, wlexer)))
            return rc;
        if ((rc = sublexers[wtoken.type](lexer, wlexer, token, &wtoken)) < 0)
            return rc;
        // fallthrough doesn't make sense here
        assert(rc != LEXER_OP_FALLTHROUGH);
        if (rc & LEXER_OP_PUSH)
            wlexer_push(&wtoken, wlexer);
        if (rc & LEXER_OP_RETURN)
            return 0;
        if (rc & LEXER_OP_CONTINUE)
            continue;
    }
}

static bool is_only_digits(struct token *tok, size_t tok_len)
{
    for (size_t i = 0; i < tok_len; i++)
        if (!isdigit(token_buf(tok)[i]))
            return false;
    return true;
}

static const char *g_keywords[] = {
#define X(Type, Val) Val,
#include <nsh_lex/keywords.defs>
#undef X
};

static int compare_keyword(const void *va, const void *vb)
{
    const char *const *a = va;
    const char *const *b = vb;
    return strcmp(*a, *b);
}

enum token_type keyword_search(const char *keyword)
{
    size_t keyword_count = ARR_SIZE(g_keywords);
    const char **search_res = bsearch(&keyword, g_keywords, keyword_count,
                                      sizeof(g_keywords[0]), compare_keyword);

    if (search_res == NULL)
        return TOK_UNDEF;

    /* TOK_KEYWORD_START_ is just before the keywords list */
    return (search_res - g_keywords) + 1 + TOK_KEYWORD_START_;
}

static nsh_err_t lexer_type_token(struct lexer *lexer, struct token *tok)
{
    nsh_err_t err;
    if (tok->type != TOK_WORD)
        return NSH_OK;

    size_t tok_len = token_size(tok) - 1;

    for (size_t i = 0; i < tok_len; i++)
        if (token_buf(tok)[i] == '\0')
            return lexer_err(lexer, "no input NUL bytes are allowed");

    if (is_only_digits(tok, tok_len)) {
        struct wtoken next_tok;
        if ((err = wlexer_peek(&next_tok, &lexer->wlexer)))
            return err;
        int ch = next_tok.ch[0];
        if (ch == '>' || ch == '<')
            tok->type = TOK_IO_NUMBER;
        return NSH_OK;
    }

    char *var_sep = strchr(token_buf(tok), '=');
    if (var_sep != NULL) {
        size_t var_name_len = var_sep - token_buf(tok);
        if (variable_name_check_string(token_buf(tok), var_name_len) == 0)
            tok->type = TOK_ASSIGNMENT_WORD;
        /* in case something has an '=' char but no valid name,
        ** preserve the current TOK_WORD value of tok->type.
        */
        return NSH_OK;
    }

    /* for name keywords, the type will first be set as TOK_NAME,
    ** then by the correct type.
    */

    if (variable_name_check_string(token_buf(tok), tok_len) == 0)
        tok->type = TOK_NAME;
    /* we can't quite return yet as some keywords aren't all letters */

    enum token_type search_type = keyword_search(token_buf(tok));
    if (search_type != TOK_UNDEF)
        tok->type = search_type;
    return NSH_OK;
}

static const char *token_repr(struct token *tok)
{
    if (tok->type == TOK_NEWLINE)
        return "<newline>";
    if (tok->type == TOK_EOF)
        return "<EOF>";
    return token_buf(tok);
}

static nsh_err_t lexer_lex(struct lexer *lexer)
{
    nsh_err_t err;
    // the lexer and the IO stream both have their own global error contexts
    struct token *res = token_alloc(lexer);
    lexer_queue_push(lexer, res);

    if ((err = lexer_lex_untyped(res, &lexer->wlexer, lexer)))
        goto err_lex_untyped;

    token_push(res, '\0');

    if ((err = lexer_type_token(lexer, res)))
        goto err_typing;

    nsh_info("token { type: %-10s repr: '%s' }", token_type_repr(res->type),
             token_repr(res));
    return NSH_OK;

err_lex_untyped:
err_typing:
    token_free(lexer_queue_pop(lexer), true);
    return err;
}

nsh_err_t lexer_lex_string(char **res, struct wlexer *wlexer)
{
    nsh_err_t err;

    struct lexer lexer = {
        .wlexer = *wlexer,
        .buffer_meta = {0},
    };
    if ((err = lexer_lex(&lexer)))
        return err;
    struct token *tok = lexer_queue_get(&lexer, 0);
    *res = token_deconstruct(tok);
    return NSH_OK;
}

nsh_err_t lexer_peek_at(const struct token **res, struct lexer *lexer, size_t i)
{
    nsh_err_t err;
    while (i >= lexer_queue_size(lexer)) {
        if ((err = lexer_lex(lexer)))
            return err;
    }

    *res = lexer_queue_get(lexer, i);
    return NSH_OK;
}

nsh_err_t lexer_peek(const struct token **res, struct lexer *lexer)
{
    return lexer_peek_at(res, lexer, 0);
}

nsh_err_t lexer_pop(struct token **res, struct lexer *lexer)
{
    nsh_err_t err;
    if (lexer_queue_size(lexer) == 0) {
        if ((err = lexer_lex(lexer)))
            return err;
    }

    *res = lexer_queue_pop(lexer);
    return NSH_OK;
}

struct lexer *lexer_create(struct cstream *stream)
{
    struct lexer *res = xmalloc(sizeof(*res));
    wlexer_init(&res->wlexer, stream);
    ring_buffer_init(&res->buffer_meta);
    return res;
}

static void lexer_reset_tokens(struct lexer *lexer)
{
    while (lexer_queue_size(lexer)) {
        struct token *tok = lexer_queue_pop(lexer);
        token_free(tok, true);
    }
}

void lexer_reset(struct lexer *lexer)
{
    lexer_reset_tokens(lexer);
    wlexer_reset(&lexer->wlexer);
}

void lexer_free(struct lexer *lexer)
{
    lexer_reset_tokens(lexer);
    free(lexer);
}

nsh_err_t lexer_discard(struct lexer *lexer)
{
    struct token *tok;
    nsh_err_t err;

    if ((err = lexer_pop(&tok, lexer)))
        return err;

    token_free(tok, true);
    return NSH_OK;
}
