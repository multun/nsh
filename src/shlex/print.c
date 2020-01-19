#include "shlex/lexer.h"
#include "shlex/print.h"
#include "utils/macros.h"


static const char *g_token_type_tab[] =
{
#define X(TokName, Value) #TokName,
#include "shlex/tokens.defs"
#undef X
};

const char *token_type_to_string(enum token_type type)
{
    if (type > ARR_SIZE(g_token_type_tab))
        return NULL;
    return g_token_type_tab[type];
}

int print_tokens(FILE *f, struct cstream *cs, struct errcont *errcont)
{
    int res = 0;
    struct lexer *lex = lexer_create(cs);
    while (!cstream_eof(cs)) {
        struct token *tok = lexer_pop(lex, errcont);
        if (!tok) {
            res = 1;
            break;
        }

        fprintf(f, "%zu:%zu\t%s(%s)[%c]\n", tok->lineinfo.line, tok->lineinfo.column,
                token_type_to_string(tok->type), tok_buf(tok), tok->delim);
        tok_free(tok, true);
    }
    lexer_free(lex);
    return res;
}
