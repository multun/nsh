#include "repl/history.h"
#include "utils/pathutils.h"

#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <readline/history.h>


void history_init(s_context *ctx)
{
  if (!ctx->cs->interactive)
  {
    ctx->history = NULL;
    return;
  }

  char *history_path = home_suffix("/.42sh_history");
  ctx->history = fopen(history_path, "a+");
  free(history_path);

  if (!ctx->history)
  {
    warnx("couldn't open history file");
    return;
  }

  if (ctx->history && fcntl(fileno(ctx->history), F_SETFD, FD_CLOEXEC) < 0)
    warn("couldn't set CLOEXEC on history file");
}


void history_update(s_context *ctx)
{
  if (!ctx->cs->interactive)
    return;


  s_evect *cmd_vect = &ctx->cs->linebuf;
  if (cmd_vect->data[ctx->cs->linebuf.size - 1] == '\n')
    cmd_vect->data[--ctx->cs->linebuf.size] = '\0';
  else
    evect_push(cmd_vect, '\0');

  add_history(cmd_vect->data);

  if (!ctx->history)
    return;

  cmd_vect->data[ctx->cs->linebuf.size - 1] = '\n';
  evect_push(cmd_vect, '\0');

  if (fputs(cmd_vect->data, ctx->history) == EOF)
  {
    warnx("couldn't update history, closing history file");
    history_destroy(ctx);
  }
  else
    ctx->cs->linebuf.size = 0;
}


void history_destroy(s_context *ctx)
{
  if (!ctx->history)
    return;

  fclose(ctx->history);
  ctx->history = NULL;
}
