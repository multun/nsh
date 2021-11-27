#include <nsh_lex/lexer.h>


struct token *token_alloc(struct lexer *lexer);


typedef enum wlexer_op (*sublexer_f)(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken);

enum wlexer_op sublexer_regular(struct lexer *lexer, struct wlexer *wlexer,
                                struct token *token, struct wtoken *wtoken);

__noreturn void lexer_err(struct lexer *lexer, const char *fmt, ...);

int lexer_lex_untyped(struct token *token, struct wlexer *wlexer, struct lexer *lexer);

extern sublexer_f sublexers[];
