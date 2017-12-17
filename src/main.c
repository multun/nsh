#include "cli/cmdopts.h"
#include "repl/repl.h"



/**
** \mainpage Introduction
**
** dish could be divided into seven main components:
**    - utils, a general toolbox
**    - cli, the command line parsing toolkit
**    - io, the base IO abstractions
**    - shlex, the shell lexer
**    - shparse, the shell parser
**    - shexp, the expression expansion toolkit
**    - shexec, the shell AST evaluation module
**    - repl, the main program loop
**
**  You may find some more detailed explanations for these components
** by clicking the links above.
**
**  Here is a basic overview of what appends:
**   - the \ref cmdopts_parse "cli module" parses the arguments,
**     stores the options and shopts
**   - the io layer \ref cstream_dispatch_init "creates a stream" based
**     on the arguments
**   - a lexer \ref lexer_create "is configured" to call the just configured
**     io layer
**   - the \ref repl "main loop" will call \ref parse "a parser", which will
**     \ref lexer_peek "call the lexer", which will call the io layer, and
**     hopefuly produce an AST.
**   - the \ref repl "main loop" \ref ast_exec "hands the ast" to the shexec
**      module, which will in turn call \ref expand "shexp" when necessary
**   - the operation is repeated as long as the stream
**     \ref cstream_eof "isn't exhausted"
**
**  In order to ensure all allocated chunks are freed,
** \ref shraise "an exception mechanism" has been implemented.
** When an error or a planned exit occurs, a chained sequence of longjmp calls
** goes up the stack and hopefuly frees all allocated data.
**
*/



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
