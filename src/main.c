#include "cli/cmdopts.h"
#include "repl/repl.h"


static int run(s_arg_context *arg_cont)
{
  s_context cont;
  int res = 0;
  bool should_exit = context_init(&res, &cont, arg_cont);

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
    return CMDOPTS_STATUS(cmdstart);
  return run(&ARG_CONTEXT(argc, cmdstart, argv));
}
