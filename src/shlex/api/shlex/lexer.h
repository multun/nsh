#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "io/cstream.h"
#include "utils/attr.h"
#include "utils/error.h"
#include "utils/evect.h"
#include "shlex/lexer_error.h"
#include "shwlex/wlexer.h"

#define TOK_BUF_MIN_SIZE 10

/**
** \brief represents a token. it features a type, a delimiter, a
**   string representation and a pointer to the next token in the stream.
*/
struct token
{
    enum token_type
    {
#define X(TokName, Value) TokName,
#include "tokens.defs"
#undef X
    } type;

    struct lineinfo lineinfo;
    int delim;
    struct evect str;
    struct token *next;
};

static inline char *tok_buf(const struct token *token)
{
    return token->str.data;
}

static inline size_t tok_size(const struct token *token)
{
    return token->str.size;
}

static inline void wtoken_push(struct token *token, struct wtoken *wtok)
{
    for (char *cur = wtok->ch; *cur; cur++) {
        assert(cur < wtok->ch + sizeof(wtok->ch));
        evect_push(&token->str, *cur);
    }
}

static inline void tok_push(struct token *token, char c)
{
    evect_push(&token->str, c);
}

/**
** \brief represents a fully featured lexer
*/
struct lexer
{
    /* the head of the token stack */
    struct token *head;
    /* the toplevel word lexer */
    struct wlexer wlexer;
    /* using a global per lexer ex_scope avoids passing it around all functions
     * inside the lexer, which doesn't create new contexts anyway */
    struct ex_scope *ex_scope;
};

static inline struct lineinfo *lexer_line_info(struct lexer *lexer)
{
    return wlexer_line_info(&lexer->wlexer);
}

typedef enum wlexer_op (*sublexer)(struct lexer *lexer, struct wlexer *wlexer,
                                  struct token *token, struct wtoken *wtoken);

enum wlexer_op sublexer_regular(struct lexer *lexer, struct wlexer *wlexer,
                               struct token *token, struct wtoken *wtoken);

__noreturn void lexer_err(struct lexer *lexer, const char *fmt, ...);

int lexer_lex_untyped(struct token *token, struct wlexer *wlexer, struct lexer *lexer);

extern sublexer sublexers[];


/**
** \brief allocates a new token
*/
struct token *tok_alloc(struct lexer *lexer);

/**
** \brief frees an allocated token
** \arg free_buf whether to free the underlying buffer
*/
void tok_free(struct token *free, bool free_buf);

/**
** \brief allocates a new lexer
** \param stream the character stream to bind the lexer to
*/
struct lexer *lexer_create(struct cstream *stream);

/**
** \brief frees a lexer
*/
void lexer_free(struct lexer *lexer);

/**
** \brief resets the lexer, which includes destroying all the pending tokens
*/
void lexer_reset(struct lexer *lexer);

/**
** \brief peeks a token without removing it from the stack
** \param lexer the lexer to peek at
** \param ex_scope the error context
** \return the next token to be read
*/
struct token *lexer_peek(struct lexer *lexer, struct ex_scope *ex_scope);

/**
** \brief peeks a token and removes it from the stack
** \param lexer the lexer to pop from
** \param ex_scope the error context
** \return the next token
*/
struct token *lexer_pop(struct lexer *lexer, struct ex_scope *ex_scope);

static inline void lexer_discard(struct lexer *lexer, struct ex_scope *ex_scope)
{
    tok_free(lexer_pop(lexer, ex_scope), true);
}

/**
** \brief peeks after an unpoped token
** \details peeking after a poped token is UB
** \param lexer the lexer to peek at
** \param tok the token to peek after
*/
struct token *lexer_peek_at(struct lexer *lexer, struct token *tok, struct ex_scope *ex_scope);

/**
** \brief read a word lexer stream and shove it into a string
** \details this is needed to parse and run subshells
** \param ex_scope the error context
** \param wlexer word lexer to pull words from
*/
char *lexer_lex_string(struct ex_scope *ex_scope, struct wlexer *wlexer);

static inline bool token_type_keyword(enum token_type type)
{
    switch (type) {
#define X(TypeName, ...) case TypeName:
#include "shlex/keywords.defs"
#undef X
        return true;
    default:
        return false;
    }
}

static inline bool token_type_name_keyword(enum token_type type)
{
    switch (type) {
#define X(TypeName, ...) case TypeName:
#include "shlex/name_keywords.defs"
#undef X
        return true;
    default:
        return false;
    }
}

/**
** \brief tests whether a token can be of the requested type
*/
static inline bool tok_is(const struct token *tok, enum token_type type)
{
    switch (type) {
    case TOK_WORD:
        return tok->type == TOK_ASSIGNMENT_WORD || tok->type == TOK_NAME
            || tok->type == TOK_WORD || token_type_keyword(tok->type);
    case TOK_NAME:
        return tok->type == TOK_NAME || token_type_name_keyword(tok->type);
    default:
        return tok->type == type;
    }
}
