#include "shlex/lexer.h"


const char *token_type_to_string(enum token_type);
int print_tokens(FILE *f, s_cstream *cs, s_errman *errman);
