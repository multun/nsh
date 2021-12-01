#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <nsh_io/cstream.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/error.h>
#include <nsh_utils/exception.h>
#include <nsh_utils/evect.h>
#include <nsh_utils/ring_buffer.h>
#include <nsh_lex/lexer_error.h>
#include <nsh_lex/wlexer.h>
#include <nsh_lex/token.h>


#define LEXER_LOOKAHEAD 2


struct lexer
{
    /* the upcoming tokens */
    struct token *next_tokens[LEXER_LOOKAHEAD];
    struct ring_buffer_meta buffer_meta;

    /* the toplevel word lexer */
    struct wlexer wlexer;
    /* using a global per lexer catcher avoids passing it around all functions
     * inside the lexer, which doesn't create new contexts anyway */
    struct exception_catcher *catcher;
};


struct lexer *lexer_create(struct cstream *stream);
void lexer_free(struct lexer *lexer);

/** \brief Resets the lexer to its initial state */
void lexer_reset(struct lexer *lexer);

/** \brief Returns the next token */
nsh_err_t lexer_peek(const struct token **res, struct lexer *lexer);
/** \brief Returns the nth next token */
nsh_err_t lexer_peek_at(const struct token **res, struct lexer *lexer, size_t i);

/** \brief Returns the next token and removes it from the queue */
nsh_err_t lexer_pop(struct token **res, struct lexer *lexer);

/** \brief Discards the next token in the queue */
nsh_err_t lexer_discard(struct lexer *lexer);


/**
** \brief read a word lexer stream and shove it into a string
** \details this is needed to parse and run subshells
** \param wlexer word lexer to pull words from
*/
nsh_err_t lexer_lex_string(char **res, struct wlexer *wlexer);

static inline struct lineinfo *lexer_line_info(struct lexer *lexer)
{
    return wlexer_line_info(&lexer->wlexer);
}
