#include "io/managed_stream.h"
#include "io/cstream.h"

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


bool init_command(struct managed_stream *ms, int argc, char *argv[])
{
  if (g_cmdopts.src != SHSRC_COMMAND)
    return false;

  if (argc == 0)
    errx(2, "missing command");

  ms->cs = cstream_from_string(argv[0], "<command line>");
  return true;
}


bool init_file(struct managed_stream *ms, int argc, char *argv[])
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
  return true;
}


void managed_stream_init(struct managed_stream *ms, int argc, char *argv[])
{
  ms->in_file = NULL;

  if (init_command(ms, argc, argv) || init_file(ms, argc, argv))
    return;

  if (is_interactive(argc))
    ms->cs = cstream_readline();
  else
    ms->cs = cstream_from_file(stdin, "<stdin>");
}


void managed_stream_destroy(struct managed_stream *ms)
{
  if (ms->in_file)
    fclose(ms->in_file);
  cstream_free(ms->cs);
}
