#include <nsh_utils/lineinfo.h>
#include <nsh_utils/evect.h>


enum token_type
{
#define X(TokName, Value) TokName,
#include "tokens.defs"
#undef X
};

/**
** \brief represents a token. it features a type, a delimiter, a
**   string representation and a pointer to the next token in the stream.
*/
struct token
{
    enum token_type type;
    struct lineinfo lineinfo;
    int delim;
    struct evect str;
};

static inline char *token_buf(const struct token *token)
{
    return token->str.data;
}

static inline size_t token_size(const struct token *token)
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

static inline void token_push(struct token *token, char c)
{
    evect_push(&token->str, c);
}

/**
** \brief frees an allocated token
** \arg free_buf whether to free the underlying buffer
*/
void token_free(struct token *free, bool free_buf);

/** \brief Returns a text representation of a token type */
const char *token_type_repr(enum token_type);


static inline char *token_deconstruct(struct token *token)
{
    char *buf = token_buf(token);
    token_free(token, false);
    return buf;
}

/** \brief Tests whether a token has the requested type, account for type inheritance */
bool token_is(const struct token *tok, enum token_type type);
