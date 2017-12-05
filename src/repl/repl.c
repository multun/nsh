#include "ast/ast_list.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "repl/repl.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/alloc.h"
#include "utils/error.h"

#include <stdio.h>
// readline's header requires including stdio beforehand
#include <readline/readline.h>
#include <readline/history.h>

#include <stdbool.h>
#include <stdlib.h>


void ast_print(FILE *f, s_ast *ast);


int repl(f_stream_consumer consumer)
{
  int res = 0;
  s_errman errman = ERRMAN;
  s_context cont;
  context_init(&cont);
  for (char *input; (input = readline("42sh> ")); free(input))
  {
    s_cstream *ns = cstream_from_string(input, "<stdin>");
    res = consumer(ns, &errman, &cont);
    cstream_free(ns);
    // TODO: handle shopt fail on error
  }
  context_destroy(&cont);
  return res;
}


void context_init(s_context *cont)
{
  cont->ast_list = NULL;
  cont->env = environment_create();
}


void context_destroy(s_context *cont)
{
  ast_list_free(cont->ast_list);
  environment_free(cont->env);
}
