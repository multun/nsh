#include <nsh_lex/lexer.h>
#include <nsh_utils/alloc.h>

#include <stdlib.h>
#include <string.h>

struct token *tok_alloc(struct lexer *lexer)
{
    struct token *res = xmalloc(sizeof(*res));
    res->lineinfo = lexer->wlexer.cs->line_info;
    res->type = TOK_WORD;
    res->next = NULL;
    evect_init(&res->str, TOK_BUF_MIN_SIZE);
    return res;
}

void tok_free(struct token *tok, bool free_buf)
{
    if (free_buf)
        evect_destroy(&tok->str);
    free(tok);
}
