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

__noreturn void lexer_err(struct lexer *lexer, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsherror(lexer_line_info(lexer), lexer->catcher, &g_lexer_error, fmt, ap);

    va_end(ap);
}

int lexer_lex_untyped(struct token *token, struct wlexer *wlexer, struct lexer *lexer)
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

static void lexer_type_token(struct lexer *lexer, struct token *tok)
{
    if (tok->type != TOK_WORD)
        return;

    size_t tok_len = token_size(tok) - 1;

    for (size_t i = 0; i < tok_len; i++)
        if (token_buf(tok)[i] == '\0')
            lexer_err(lexer, "no input NUL bytes are allowed");

    if (is_only_digits(tok, tok_len)) {
        struct wtoken next_tok;
        wlexer_peek(&next_tok, &lexer->wlexer);
        int ch = next_tok.ch[0];
        if (ch == '>' || ch == '<')
            tok->type = TOK_IO_NUMBER;
        return;
    }

    char *var_sep = strchr(token_buf(tok), '=');
    if (var_sep != NULL) {
        size_t var_name_len = var_sep - token_buf(tok);
        if (variable_name_check_string(token_buf(tok), var_name_len) == 0)
            tok->type = TOK_ASSIGNMENT_WORD;
        /* in case something has an '=' char but no valid name,
        ** preserve the current TOK_WORD value of tok->type.
        */
        return;
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
}

static const char *token_repr(struct token *tok)
{
    if (tok->type == TOK_NEWLINE)
        return "<newline>";
    if (tok->type == TOK_EOF)
        return "<EOF>";
    return token_buf(tok);
}

static void lexer_lex(struct lexer *lexer, struct exception_catcher *catcher)
{
    // the lexer and the IO stream both have their own global error contexts
    lexer->catcher = catcher;
    wlexer_set_catcher(&lexer->wlexer, catcher);

    struct token *res = token_alloc(lexer);
    lexer_queue_push(lexer, res);
    lexer_lex_untyped(res, &lexer->wlexer, lexer);
    token_push(res, '\0');

    lexer_type_token(lexer, res);

    nsh_info("token { type: %-10s repr: '%s' }", token_type_repr(res->type),
             token_repr(res));
}

static nsh_err_t compat_lexer_lex(struct lexer *lexer)
{
    EXCEPTION_COMPAT_STUB(lexer_lex(lexer, compat_catcher));
}

char *lexer_lex_string(struct exception_catcher *catcher, struct wlexer *wlexer)
{
    struct lexer lexer = {
        .wlexer = *wlexer,
        .catcher = catcher,
        .buffer_meta = {0},
    };

    lexer_lex(&lexer, catcher);
    struct token *tok = lexer_queue_get(&lexer, 0);
    return token_deconstruct(tok);
}

nsh_err_t lexer_peek_at(const struct token **res, struct lexer *lexer, size_t i)
{
    nsh_err_t err;
    while (i >= lexer_queue_size(lexer)) {
        if ((err = compat_lexer_lex(lexer)))
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
        if ((err = compat_lexer_lex(lexer)))
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
