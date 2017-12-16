#include "cli/cmdopts.h"
#include "repl/repl.h"

#include <stdio.h>


static int run(int argc, char *argv[])
{
  s_context cont;
  int res = 0;
  bool should_exit = context_init(&res, &cont, argc, argv);

  if (!should_exit)
  {
    // the return value represents whether
    // the repl exited using an exception
    repl(&cont);
    res = cont.env->code;
  }

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

  return run(argc, argv);
}
