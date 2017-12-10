#include "cli/cmdopts.h"
#include "gen/config.h"
#include "repl/repl.h"

#include <stdio.h>


static int run(int argc, char *argv[])
{
  s_context cont;
  context_init(&cont, argv);

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
