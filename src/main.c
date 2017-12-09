#include "cli/cmdopts.h"
#include "gen/config.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"
#include "repl/repl.h"
#include "shparse/parse.h"

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/error.h"
#include "shexec/clean_exit.h"


/* static int ast_print_consumer(s_cstream *cs, s_errcont *errcont, */
/*                               s_context *cont) */
/* { */
/*   s_lexer *lex = lexer_create(cs); */
/*   s_ast *ast = NULL; */
/*   parse(&ast, lex, errcont); */

/*   // TODO: catch exceptions here */

/*   cont->ast_list = ast_list_append(cont->ast_list, ast); */
/*   FILE *f = fopen("42sh_ast.dot", "w+"); */
/*   ast_print(f, ast); */
/*   fclose(f); */
/*   return 0; */
/* } */


/* static int token_print_consumer(s_cstream *cs, s_errcont *errcont, */
/*                                 s_context *cont) */
/* { */
/*   if (!cont) */
/*     return 0; */
/*   return print_tokens(stdout, cs, errcont); */
/* } */


static void try_re(int *res, s_lexer *lex,
                   s_errcont *errcont, s_context *cont)
{
  parse(&cont->ast, lex, errcont);
  if (cont->ast)
    *res = ast_exec(cont->env, cont->ast, errcont);
}


static bool handle_rep_fail(int *res, s_errman *eman)
{
  if (eman->class == &g_clean_exit)
  {
    *res = eman->retcode;
    return true;
  }
  *res = 2;
  return false;
}


static bool ast_exec_consumer(int *res, s_lexer *lex, s_context *cont)
{
  cont->line_start = true;
  cont->ast = NULL;

  s_errman eman = ERRMAN;
  s_keeper keeper = KEEPER(NULL);

  bool stopping = false;
  if (setjmp(keeper.env))
  {
    if (handle_rep_fail(res, &eman))
      stopping = true;
  }
  else
    try_re(res, lex, &ERRCONT(&eman, &keeper), cont);

  cont->ast_list = ast_list_append(cont->ast_list, cont->ast);
  return stopping;
}


static int producer(struct context *ctx, int argc, char *argv[])
{
  struct managed_stream ms;
  managed_stream_init(ctx, &ms, argc, argv);

  int res = 0;
  for (bool stopping = false; !stopping && !cstream_eof(ms.cs);)
  {
    s_lexer *lex = lexer_create(ms.cs);
    stopping = ast_exec_consumer(&res, lex, ctx);
    lexer_free(lex);
  }

  managed_stream_destroy(&ms);
  return res;
}


static int run(int argc, char *argv[])
{
  s_context cont;
  context_init(&cont);

  if (g_cmdopts.shmode == SHMODE_VERSION)
  {
    puts("Version " VERSION);
    return 0;
  }

  int res = producer(&cont, argc, argv);

  context_destroy(&cont);
  return res;
}


int main(int argc, char *argv[])
{
  int cmdstart = cmdopts_parse(argc, argv);
  if (cmdstart < 0)
    // exit when cmdstart < 0,
    // but succeed when cmdstart == -1
    return cmdstart + 1;

  argc -= cmdstart;
  argv += cmdstart;

  if (g_cmdopts.norc)
    puts("norc");

  return run(argc, argv);
}
