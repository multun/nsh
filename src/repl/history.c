#include "repl/history.h"

#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>


void history_init(s_context *ctx)
{
  if (!ctx->ms.cs->interactive)
  {
    ctx->history = NULL;
    return;
  }

  // TODO: set proper path limit
  char history_path[512];
  strcat(strcpy(history_path, getpwuid(getuid())->pw_dir), "/.42sh_history");

  if (!(ctx->history = fopen(history_path, "a+")))
  {
    warnx("couldn't open history file");
    return;
  }

  if (ctx->history && fcntl(fileno(ctx->history), F_SETFD, FD_CLOEXEC) < 0)
    warn("couldn't set CLOEXEC on history file");
}


void history_update(s_context *ctx)
{
  if (!ctx->history || !ctx->ms.cs->interactive)
    return;


  evect_push(&ctx->ms.cs->linebuf, '\0');
  const char *command = ctx->ms.cs->linebuf.data;
  if (fputs(command, ctx->history) == EOF)
  {
    warnx("couldn't update history, closing history file");
    history_destroy(ctx);
  }
  else
    ctx->ms.cs->linebuf.size = 0;
}


void history_destroy(s_context *ctx)
{
  if (!ctx->history)
    return;

  fclose(ctx->history);
  ctx->history = NULL;
}
