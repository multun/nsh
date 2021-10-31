#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/history.h>
#include <nsh_exec/ast_exec.h>
#include <nsh_exec/clean_exit.h>

#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


void repl_init(struct repl *ctx, struct cstream *cs, struct environment *env)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->cs = cs;
    ctx->env = env;
    ctx->lexer = lexer_create(cs);
    environment_get(env);
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
