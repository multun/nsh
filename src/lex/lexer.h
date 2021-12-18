#include <nsh_lex/lexer.h>


struct token *token_alloc(struct lexer *lexer);


typedef int (*sublexer_f)(struct lexer *lexer, struct wlexer *wlexer, struct token *token,
                          struct wtoken *wtoken);

int sublexer_regular(struct lexer *lexer, struct wlexer *wlexer, struct token *token,
                     struct wtoken *wtoken);

nsh_err_t lexer_err(struct lexer *lexer, const char *fmt, ...) __unused_result;

int lexer_lex_untyped(struct token *token, struct wlexer *wlexer, struct lexer *lexer);

extern sublexer_f sublexers[];
