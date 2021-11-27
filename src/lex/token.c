#include <nsh_lex/lexer.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/macros.h>

#include <stdlib.h>
#include <string.h>

#define TOK_BUF_INIT_SIZE 10


struct token *token_alloc(struct lexer *lexer)
{
    struct token *res = xmalloc(sizeof(*res));
    res->lineinfo = lexer->wlexer.cs->line_info;
    res->type = TOK_WORD;
    evect_init(&res->str, TOK_BUF_INIT_SIZE);
    return res;
}

void token_free(struct token *tok, bool free_buf)
{
    if (free_buf)
        evect_destroy(&tok->str);
    free(tok);
}

static const char *g_token_type_tab[] = {
#define X(TokName, Value) #TokName,
#include <nsh_lex/tokens.defs>
#undef X
};

const char *token_type_repr(enum token_type type)
{
    if (type > ARR_SIZE(g_token_type_tab))
        return NULL;
    return g_token_type_tab[type];
}

static bool token_type_keyword(enum token_type type)
{
    switch (type) {
#define X(TypeName, ...) case TypeName:
#include <nsh_lex/keywords.defs>
#undef X
        return true;
    default:
        return false;
    }
}

static bool token_type_name_keyword(enum token_type type)
{
    switch (type) {
#define X(TypeName, ...) case TypeName:
#include <nsh_lex/name_keywords.defs>
#undef X
        return true;
    default:
        return false;
    }
}

bool token_is(const struct token *tok, enum token_type type)
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
