#include "cli/cmdopts.h"
#include "gen/config.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"
#include "repl/repl.h"
#include "shparse/parse.h"
#include "repl/history.h"

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/error.h"
#include "shexec/clean_exit.h"


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
    return -(cmdstart + 1);

  argc -= cmdstart;
  argv += cmdstart;

  if (g_cmdopts.norc)
    puts("norc");

  return run(argc, argv);
}
