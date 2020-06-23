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

    /* if we couln't open an history file, reset the buffer and give up */
    if (!ctx->history) {
        evect_reset(cmd_vect);
        return;
    }

    /* there should be something in the command buffer, as parsing
       must have proceeded for history_update to be called */
    assert(cmd_vect->size != 0);

    /* in production, just skip adding to the history if it happens anyway */
    if (cmd_vect->size == 0)
        return;

    /* NUL-terminate the command buffer, removing that one anoying \n if it's there */
    if (cmd_vect->data[cmd_vect->size - 1] == '\n')
        cmd_vect->data[--cmd_vect->size] = '\0';
    else
        evect_push(cmd_vect, '\0');

    /* use readline_history */
    add_history(cmd_vect->data);

    if (fputs(cmd_vect->data, ctx->history) == EOF) {
        warnx("couldn't update history, closing history file");
        fclose(ctx->history);
        ctx->history = NULL;
    } else
        evect_reset(cmd_vect);
}

void history_destroy(struct context *ctx)
{
    if (!ctx->cs->interactive)
        return;

    evect_destroy(&ctx->line_buffer);

    if (ctx->history) {
        fclose(ctx->history);
        ctx->history = NULL;
    }
}
