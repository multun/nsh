#include <nsh_lex/lexer.h>

#define TOKT_STR(Tok) (token_type_to_string(Tok->type))

/**
** \brief retrieves the string representation of a type
*/
const char *token_type_repr(enum token_type);
