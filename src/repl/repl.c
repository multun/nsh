#include "ast/ast_list.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "repl/repl.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/alloc.h"
#include "utils/error.h"

#include <string.h>
#include <stdio.h>
// readline's header requires including stdio beforehand
#include <readline/history.h>
#include <readline/readline.h>

#include <err.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


void ast_print(FILE *f, s_ast *ast);


int repl(f_stream_consumer consumer)
{
  int res = 0;
  s_errman errman;
  s_context cont;
  context_init(&cont);
  char history_path[512];
  strcat(strcpy(history_path, getpwuid(getuid())->pw_dir), "/.42sh_history");

  FILE *history = fopen(history_path, "a+");

  if (history && fcntl(fileno(history), F_SETFD, FD_CLOEXEC) < 0)
    errx(1, "42sh: repl: Failed CLOEXEC file descriptor %d", fileno(history));

  for (char *input; (input = readline("42sh> ")); free(input))
  {
    errman = ERRMAN;
    s_cstream *ns = cstream_from_string(input, "<stdin>");
    res = consumer(ns, &errman, &cont);
    if (history && !ERRMAN_FAILING(&errman))
      fprintf(history, "%s\n", input);
    cstream_free(ns);
    // TODO: handle shopt fail on error
  }
  if (history)
    fclose(history);
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
