#include <nsh_lex/lexer.h>

#define TOKT_STR(Tok) (token_type_to_string(Tok->type))

/**
** \brief retrieves the string representation of a type
*/
const char *token_type_to_string(enum token_type);

/**
** \brief pops all tokens from a stream until EOF, and pretty-print these
** \param f the file to print to
** \param cs the stream to read characters from
** \param ex_scope the error context to fail into
** \return the resulting status code
*/
int print_tokens(FILE *f, struct cstream *cs, struct ex_scope *ex_scope);
