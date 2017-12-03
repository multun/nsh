#include "io/cstream.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"


#include <stdio.h>
// readline's header requires including stdio beforehand
#include <readline/readline.h>
#include <readline/history.h>

#include <stdbool.h>
#include <stdlib.h>


void ast_print(FILE *f, s_ast *ast);


int repl(s_cstream *cs, int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  (void)cs;
  for (char *input; (input = readline("42sh> ")); free(input))
  {
    s_cstream *ns = cstream_from_string(input, "<stdin>");
    s_lexer *lex = lexer_create(ns);
    s_ast *ast = parse(lex);
    ast_print(stdout, ast);
    cstream_free(ns);
  }
  return 0;
}
