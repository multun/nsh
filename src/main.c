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


static bool is_interactive(int argc)
{
  return (g_cmdopts.src != SHSRC_COMMAND) && argc <= 0 && isatty(STDIN_FILENO);
}


static int ast_print_consumer(s_cstream *cs, s_errman *errman)
{
  s_lexer *lex = lexer_create(cs);
  s_ast *ast = parse(lex, errman);
  if (ERRMAN_FAILING(errman))
  {
    ast_free(ast);
    return 1;
  }
  if (!ast)
    return 1;
  ast_print(stdout, ast);
  ast_free(ast);
  return 0;
}


static int token_print_consumer(s_cstream *cs, s_errman *errman)
{
  return print_tokens(stdout, cs, errman);
}


static int ast_exec_consumer(s_cstream *cs, s_errman *errman)
{
  s_lexer *lex = lexer_create(cs);
  s_ast *ast = parse(lex, errman);
  if (!ast || ERRMAN_FAILING(errman))
  {
    ast_free(ast);
    lexer_free(lex);
    return 1;
  }
  ast_exec(NULL, ast);
  ast_free(ast);
  lexer_free(lex);
  return 0;
}


static int producer(f_stream_consumer consumer,
                    int cmdstart, int argc, char *argv[])
{
  (void)cmdstart; // TODO: pass args to exec
  struct managed_stream ms;
  int load_res = managed_stream_init(&ms, argc, argv);
  if (load_res)
    return load_res;

  s_errman errman = ERRMAN;

  int res = 0;
  while (!cstream_eof(ms.cs) && !ERRMAN_FAILING(&errman))
    res = consumer(ms.cs, &errman);

  managed_stream_destroy(&ms);
  return res;
}


static int run(int cmdstart, int argc, char *argv[])
{
  f_stream_consumer consumer = NULL;

  switch (g_cmdopts.shmode)
  {
  case SHMODE_VERSION:
    puts("Version " VERSION);
    return 0;
  case SHMODE_AST_PRINT:
    consumer = ast_print_consumer;
    break;
  case SHMODE_TOKEN_PRINT:
    consumer = token_print_consumer;
    break;
  case SHMODE_REGULAR:
    consumer = ast_exec_consumer;
    break;
  }

  if (is_interactive(argc))
    return repl(consumer);
  return producer(consumer, cmdstart, argc, argv);
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

  return run(cmdstart, argc, argv);
}
