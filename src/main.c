#include "cli/cmdopts.h"
#include "gen/config.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"
#include "repl/repl.h"

#include <stdlib.h>


static int run(struct managed_stream *ms, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  switch (g_cmdopts.shmode)
  {
  case SHMODE_VERSION:
    puts("Version " VERSION);
    return 0;
  case SHMODE_AST_PRINT:
    return 1;
  case SHMODE_TOKEN_PRINT:
    return print_tokens(stdout, ms->cs);
  case SHMODE_REGULAR:
    return repl(ms->cs, argc, argv);
  }
  abort();
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

  struct managed_stream ms;
  int load_res = managed_stream_init(&ms, argc, argv);
  if (load_res)
    return load_res;

  int res = run(&ms, argc, argv);

  managed_stream_destroy(&ms);
  return res;
}
