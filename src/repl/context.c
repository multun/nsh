#include "cli/cmdopts.h"
#include "io/cstream.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/environment.h"
#include "utils/pathutils.h"

#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool context_load_rc(struct environment *env, const char *path, const char *source)
{
    struct context ctx;

    FILE *file = fopen(path, "r");
    int res;
    if ((res = cstream_file_setup(&file, path, true)))
        return 0; // not being able to load an rc file is ok

    struct cstream_file cs;
    cstream_file_init(&cs, file, true);
    cs.base.line_info = LINEINFO(source, NULL);
    cs.base.context = &ctx;

    context_from_env(&ctx, &cs.base, env);

    bool should_exit = repl(&ctx);

    context_destroy(&ctx);
    cstream_destroy(ctx.cs);
    return should_exit;
}

static bool context_load_all_rc(struct context *ctx)
{
    const char global_rc[] = "/etc/nshrc";
    if (context_load_rc(ctx->env, global_rc, global_rc))
        return true;

    char *rc_path = home_suffix("/.nshrc");
    bool should_exit = context_load_rc(ctx->env, rc_path, "~/.nshrc");
    free(rc_path);
    return should_exit;
}

void context_from_env(struct context *ctx, struct cstream *cs, struct environment *env)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->cs = cs;
    ctx->env = env;
    ctx->lexer = lexer_create(cs);
    environment_get(env);
}

bool context_init(int *rc, struct context *ctx, struct cstream *cs, struct arg_context *arg_ctx)
{
    struct environment *env = environment_create(arg_ctx);
    context_from_env(ctx, cs, env);
    environment_put(env);

    if (ctx->cs->interactive && !g_cmdopts.norc && context_load_all_rc(ctx)) {
        *rc = ctx->env->code;
        return true;
    }

    history_init(ctx);
    return false;
}

void context_destroy(struct context *ctx)
{
    history_destroy(ctx);
    lexer_free(ctx->lexer);
    environment_put(ctx->env);
}
