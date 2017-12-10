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


static int init_command(struct managed_stream *ms, int argc, char *argv[])
{
  if (argc == 0)
  {
    warnx("missing command");
    return 2;
  }

  ms->cs = cstream_from_string(argv[0], "<command line>");
  ms->cs->interactive = false;
  ms->cs->context = NULL;
  return 0;
}


static int init_file(struct managed_stream *ms, int argc, char *argv[])
{
  if (argc < 1)
    return false;

  if (!(ms->in_file = fopen(argv[0], "r")))
    // TODO: check the return code is right
    errx(errno, "cannot open input script: %s\n", strerror(errno));

  if (fcntl(fileno(ms->in_file), F_SETFD, FD_CLOEXEC) < 0)
    errx(errno, "couldn't set CLOEXEC on input file %d",
         fileno(ms->in_file));
  ms->cs = cstream_from_file(ms->in_file, argv[0]);
  ms->cs->interactive = false;
  ms->cs->context = NULL;
  return true;
}


int managed_stream_init(struct context *context, struct managed_stream *ms,
                        int argc, char *argv[])
{
  ms->in_file = NULL;
  ms->cs = NULL;

  if (g_cmdopts.src == SHSRC_COMMAND)
    return init_command(ms, argc, argv);


  if (argc >= 1)
    return init_file(ms, argc, argv);


  if (is_interactive(argc))
  {
    ms->cs = cstream_readline();
    ms->cs->interactive = true;
    ms->cs->context = context;
  }
  else
  {
    ms->cs = cstream_from_file(stdin, "<stdin>");
    ms->cs->interactive = false;
    ms->cs->context = NULL;
  }
  return 0;
}


void managed_stream_destroy(struct managed_stream *ms)
{
  if (ms->in_file)
    fclose(ms->in_file);
  cstream_free(ms->cs);
}
