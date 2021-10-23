#include "io/cstream.h"
#include "shexec/history.h"
#include "shexec/repl.h"
#include "shexec/environment.h"
#include "utils/pathutils.h"
#include "shparse/ast.h"
#include "shexec/clean_exit.h"

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

    struct repl_result repl_res;
    repl(&repl_res, &ctx);

    context_destroy(&ctx);
    cstream_destroy(ctx.cs);

    /* exiting in an rc file causes the shell to exit */
    return repl_called_exit(&repl_res);
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

bool context_init(int *rc, struct context *ctx, struct cstream *cs, struct cli_options *arg_ctx)
{
    struct environment *env = environment_create(arg_ctx);
    context_from_env(ctx, cs, env);
    environment_put(env);

    if (ctx->cs->interactive && !arg_ctx->norc && context_load_all_rc(ctx)) {
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
    context_drop_ast(ctx);
}

void context_reset(struct context *ctx)
{
    ctx->line_start = true;
    cstream_reset(ctx->cs);
    lexer_reset(ctx->lexer);
    evect_reset(&ctx->line_buffer);
    context_drop_ast(ctx);
}

void context_drop_ast(struct context *ctx)
{
    if (!ctx->ast)
        return;

    shast_ref_put(ctx->ast);
    ctx->ast = NULL;
}
