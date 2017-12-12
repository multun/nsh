#include "io/managed_stream.h"
#include "io/cstream.h"
#include "repl/repl.h"
#include "cli/cmdopts.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


static bool is_interactive(int argc)
{
  return (g_cmdopts.src != SHSRC_COMMAND) && argc <= 0 && isatty(STDIN_FILENO);
}


static int init_command(s_cstream **cs, int argc, char *path)
{
  if (argc == 0)
  {
    warnx("missing command");
    return 2;
  }

  *cs = cstream_from_string(path, "<command line>");
  (*cs)->interactive = false;
  (*cs)->context = NULL;
  return 0;
}


static int init_file(s_cstream **cs, char *path)
{
  FILE *file;
  if (!(file = fopen(path, "r")))
  {
    int res = errno;
    // TODO: check the return code is right
    warn("cannot open input script");
    return res;
  }

  if (fcntl(fileno(file), F_SETFD, FD_CLOEXEC) < 0)
  {
    int res = errno;
    warn("couldn't set CLOEXEC on input file %d", fileno(file));
    return res;
  }

  *cs = cstream_from_file(file, path, true);
  (*cs)->interactive = false;
  (*cs)->context = NULL;
  return 0;
}


int cstream_dispatch_init(struct context *context, s_cstream **cs,
                          int argc, char *argv[])
{
  if (g_cmdopts.src == SHSRC_COMMAND)
    return init_command(cs, argc, argv[0]);


  if (argc >= 1)
    return init_file(cs, argv[0]);


  if (is_interactive(argc))
  {
    *cs = cstream_readline();
    (*cs)->interactive = true;
    (*cs)->context = context;
  }
  else
  {
    *cs = cstream_from_file(stdin, "<stdin>", false);
    (*cs)->interactive = false;
    (*cs)->context = NULL;
  }
  return 0;
}
