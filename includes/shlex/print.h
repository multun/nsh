#include "shlex/lexer.h"

#define TOKT_STR(Tok) (token_type_to_string(Tok->type))



/**
** \brief retrieves the string representation of a type
*/
const char *token_type_to_string(enum token_type);

/**
** \brief pops all tokens from a stream until EOF, and pretty-print these
*/
int print_tokens(FILE *f, s_cstream *cs, s_errman *errman);
