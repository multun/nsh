#include "cli/cmdopts.h"
#include "gen/config.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"



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

  int res = 0;
  switch (g_cmdopts.shmode)
  {
  case SHMODE_VERSION:
    puts("Version " VERSION);
    res = 0;
    break;
  case SHMODE_AST_PRINT:
    res = 1;
    break;
  case SHMODE_TOKEN_PRINT:
    res = print_tokens(stdout, ms.cs);
    break;
  case SHMODE_REGULAR:
    res = 3;
    break;
  }

  managed_stream_destroy(&ms);
  return res;
}
