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


FILE *history_open(void)
{
  char *history_path = home_suffix("/.42sh_history");
  FILE *ret = fopen(history_path, "a+");
  free(history_path);

  if (!ret)
  {
    warnx("couldn't open history file");
    return NULL;
  }

  if (ret && fcntl(fileno(ret), F_SETFD, FD_CLOEXEC) < 0)
    warn("couldn't set CLOEXEC on history file");
  return ret;
}


void history_init(s_context *ctx)
{
  if (!ctx->cs->interactive)
  {
    ctx->history = NULL;
    return;
  }
  ctx->history = history_open();
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
  {
    ctx->cs->linebuf.size = 0;
    return;
  }

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
