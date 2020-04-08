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
    char *history_path = home_suffix("/.nsh_history");
    FILE *ret = fopen(history_path, "a+");
    free(history_path);

    if (!ret) {
        warnx("couldn't open history file");
        return NULL;
    }

    if (ret && fcntl(fileno(ret), F_SETFD, FD_CLOEXEC) < 0)
        warn("couldn't set CLOEXEC on history file");
    return ret;
}

void history_init(struct context *ctx)
{
    if (!ctx->cs->interactive) {
        ctx->history = NULL;
        return;
    }

    // TODO: make this configurable
    evect_init(&ctx->line_buffer, 100);
    ctx->history = history_open();
}

void history_update(struct context *ctx)
{
    if (!ctx->cs->interactive)
        return;

    struct evect *cmd_vect = &ctx->line_buffer;
    if (cmd_vect->data[ctx->line_buffer.size - 1] == '\n')
        cmd_vect->data[--ctx->line_buffer.size] = '\0';
    else
        evect_push(cmd_vect, '\0');

    add_history(cmd_vect->data);

    if (!ctx->history) {
        ctx->line_buffer.size = 0;
        return;
    }

    cmd_vect->data[ctx->line_buffer.size - 1] = '\n';
    evect_push(cmd_vect, '\0');

    if (fputs(cmd_vect->data, ctx->history) == EOF) {
        warnx("couldn't update history, closing history file");
        history_destroy(ctx);
    } else
        ctx->line_buffer.size = 0;
}

void history_destroy(struct context *ctx)
{
    if (!ctx->history)
        return;

    evect_destroy(&ctx->line_buffer);
    fclose(ctx->history);
    ctx->history = NULL;
}
