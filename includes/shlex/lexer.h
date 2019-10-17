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

#define LEXERROR(LineInfo, Errcont, ...)                                                 \
    sherror((LineInfo), (Errcont), &g_lexer_error, __VA_ARGS__)

#define TOK_IS_OP(Type) ((Type) >= TOK_LESS && (Type) <= TOK_LESSDASH)
#define LEX_OP_TOKS(F)                                                                   \
    F(TOK_LESS, "<")                                                                     \
    F(TOK_GREAT, ">")                                                                    \
    F(TOK_SEMI, ";")                                                                     \
    F(TOK_AND, "&")                                                                      \
    F(TOK_PIPE, "|")                                                                     \
                                                                                         \
    F(TOK_LPAR, "(")                                                                     \
    F(TOK_RPAR, ")")                                                                     \
                                                                                         \
    F(TOK_AND_IF, "&&")                                                                  \
    F(TOK_OR_IF, "||")                                                                   \
    F(TOK_DSEMI, ";;")                                                                   \
    F(TOK_DLESS, "<<")                                                                   \
    F(TOK_DGREAT, ">>")                                                                  \
    F(TOK_LESSAND, "<&")                                                                 \
    F(TOK_GREATAND, ">&")                                                                \
    F(TOK_LESSGREAT, "<>")                                                               \
    F(TOK_CLOBBER, ">|")                                                                 \
    F(TOK_LESSDASH, "<<-")

#define TOK_IS_KW(Type) ((Type) >= TOK_IF && (Type) <= TOK_FUNC)
#define TOK_KW_ALIGN(Type) ((Type)-TOK_IF)

#define LEX_KW_TOKS(F)                                                                   \
    F(TOK_IF, "if")                                                                      \
    F(TOK_THEN, "then")                                                                  \
    F(TOK_ELSE, "else")                                                                  \
    F(TOK_ELIF, "elif")                                                                  \
    F(TOK_FI, "fi")                                                                      \
    F(TOK_DO, "do")                                                                      \
    F(TOK_DONE, "done")                                                                  \
                                                                                         \
    F(TOK_CASE, "case")                                                                  \
    F(TOK_ESAC, "esac")                                                                  \
    F(TOK_WHILE, "while")                                                                \
    F(TOK_UNTIL, "until")                                                                \
    F(TOK_FOR, "for")                                                                    \
    F(TOK_LBRACE, "{")                                                                   \
    F(TOK_RBRACE, "}")                                                                   \
    F(TOK_BANG, "!")                                                                     \
    F(TOK_IN, "in")                                                                      \
                                                                                         \
    F(TOK_FUNC, "function")

#define LEX_CONST_ENUM(TokName, Value) TokName,

#define TOK_IS_GEN_DET(Type)                                                             \
    ((Type) == TOK_NEWLINE || (Type) == TOK_EOF || (Type) == TOK_IO_NUMBER)
#define LEX_GEN_TOKS(F)                                                                  \
    F(TOK_WORD)                                                                          \
    F(TOK_ASSIGNMENT_WORD)                                                               \
    F(TOK_NAME)                                                                          \
    F(TOK_NEWLINE)                                                                       \
    F(TOK_IO_NUMBER)                                                                     \
                                                                                         \
    F(TOK_EOF)

#define TOK_IS_DET(Type) (TOK_IS_GEN_DET(Type) || TOK_IS_OP(Type))

#define LEX_GEN_TOKS_ENUM(Tokname) Tokname,

#define TOK_BUF_MIN_SIZE 10

/**
** \brief represents a token. it features a type, a delimiter, a
**   string representation and a pointer to the next token in the stream.
*/
struct token
{
    enum token_type
    {
        LEX_OP_TOKS(LEX_CONST_ENUM) LEX_KW_TOKS(LEX_CONST_ENUM)
            LEX_GEN_TOKS(LEX_GEN_TOKS_ENUM)
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
    /* using a global per lexer errcont avoids passing it around all functions
     * inside the lexer, which doesn't create new contexts anyway */
    struct errcont *errcont;
};

static inline struct lineinfo *lexer_line_info(struct lexer *lexer)
{
    return wlexer_line_info(&lexer->wlexer);
}

typedef enum wlexer_op (*sublexer)(struct lexer *lexer, struct wlexer *wlexer,
                                  struct token *token, struct wtoken *wtoken);

enum wlexer_op sublexer_regular(struct lexer *lexer, struct wlexer *wlexer,
                               struct token *token, struct wtoken *wtoken);

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
** \brief tests whether a token can be of the requested type
*/
bool tok_is(const struct token *tok, enum token_type type);

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
** \brief peeks a token without removing it from the stack
** \param lexer the lexer to peek at
** \param errcont the error context
** \return the next token to be read
*/
struct token *lexer_peek(struct lexer *lexer, struct errcont *errcont);

/**
** \brief peeks a token and removes it from the stack
** \param lexer the lexer to pop from
** \param errcont the error context
** \return the next token
*/
struct token *lexer_pop(struct lexer *lexer, struct errcont *errcont);

/**
** \brief peeks after an unpoped token
** \details peeking after a poped token is UB
** \param lexer the lexer to peek at
** \param tok the token to peek after
*/
struct token *lexer_peek_at(struct lexer *lexer, struct token *tok, struct errcont *errcont);

/**
** \brief read a word lexer stream and shove it into a string
** \details this is needed to parse and run subshells
** \param errcont the error context
** \param wlexer word lexer to pull words from
*/
char *lexer_lex_string(struct errcont *errcont, struct wlexer *wlexer);
