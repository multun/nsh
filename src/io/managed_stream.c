#include "io/managed_stream.h"
#include "io/cstream.h"

#include "cli/cmdopts.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


int managed_stream_init(struct managed_stream *ms, int argc, char *argv[])
{
  ms->in_file = NULL;

  if (g_cmdopts.src == SHSRC_COMMAND)
  {
    if (argc < 1)
      errx(1, "missing source");
    ms->cs = cstream_from_string(argv[0], "<command line>");
    return 0;
  }

  char *msg = "<stdin>";
  FILE *f = stdin;

  if (argc > 0)
  {
    msg = argv[0];
    if (!(f = ms->in_file = fopen(argv[0], "r")))
    {
      fprintf(stderr, "cannot open input script: %s\n", strerror(errno));
      return 1;
    }
    if (fcntl(fileno(f), F_SETFD, FD_CLOEXEC) < 0)
      errx(1, "42sh: managed_stream_init: Failed CLOEXEC file descriptor %d",
           fileno(f));
  }
  ms->cs = cstream_from_file(f, msg);
  return 0;
}


void managed_stream_destroy(struct managed_stream *ms)
{
  if (ms->in_file)
    fclose(ms->in_file);
  cstream_free(ms->cs);
}
