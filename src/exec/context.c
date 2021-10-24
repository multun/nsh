#include <nsh_io/cstream.h>
#include <nsh_exec/history.h>
#include <nsh_exec/repl.h>
#include <nsh_exec/environment.h>
#include <nsh_utils/pathutils.h>
#include <nsh_exec/ast_exec.h>
#include <nsh_exec/clean_exit.h>

#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool context_load_rc(struct environment *env, const char *path, const char *source)
{
    struct repl ctx;

    FILE *file = fopen(path, "r");
    int res;
    if ((res = cstream_file_setup(&file, path, true)))
        return 0; // not being able to load an rc file is ok

    struct cstream_file cs;
    cstream_file_init(&cs, file, true);
    cs.base.line_info = LINEINFO(source, NULL);
    cs.base.context = &ctx;

    repl_init_from_env(&ctx, &cs.base, env);

    struct repl_result repl_res;
    repl_run(&repl_res, &ctx);

    repl_destroy(&ctx);
    cstream_destroy(ctx.cs);

    /* exiting in an rc file causes the shell to exit */
    return repl_called_exit(&repl_res);
}

static bool context_load_all_rc(struct repl *ctx)
{
    const char global_rc[] = "/etc/nshrc";
    if (context_load_rc(ctx->env, global_rc, global_rc))
        return true;

    char *rc_path = home_suffix("/.nshrc");
    bool should_exit = context_load_rc(ctx->env, rc_path, "~/.nshrc");
    free(rc_path);
    return should_exit;
}

void repl_init_from_env(struct repl *ctx, struct cstream *cs, struct environment *env)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->cs = cs;
    ctx->env = env;
    ctx->lexer = lexer_create(cs);
    environment_get(env);
}

bool repl_init(int *rc, struct repl *ctx, struct cstream *cs, struct cli_options *arg_ctx)
{
    struct environment *env = environment_create(arg_ctx);
    repl_init_from_env(ctx, cs, env);
    environment_put(env);

    if (ctx->cs->interactive && !arg_ctx->norc && context_load_all_rc(ctx)) {
        *rc = ctx->env->code;
        return true;
    }

    history_init(ctx);
    return false;
}

void repl_destroy(struct repl *ctx)
{
    history_destroy(ctx);
    lexer_free(ctx->lexer);
    environment_put(ctx->env);
    repl_drop_ast(ctx);
}

void repl_reset(struct repl *ctx)
{
    ctx->line_start = true;
    cstream_reset(ctx->cs);
    lexer_reset(ctx->lexer);
    evect_reset(&ctx->line_buffer);
    repl_drop_ast(ctx);
}

void repl_drop_ast(struct repl *ctx)
{
    if (!ctx->ast)
        return;

    shast_ref_put(ctx->ast);
    ctx->ast = NULL;
}
