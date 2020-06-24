#include "io/keyboard_interrupt.h"
#include "cli/cmdopts.h"
#include "cli/shopt.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shexec/runtime_error.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/error.h"

#include <err.h>

enum repl_action
{
    REPL_ACTION_NONE = 0,
    REPL_ACTION_STOP,
    REPL_ACTION_CONTINUE,
};

static enum repl_action handle_repl_exception(struct repl_result *res, struct context *ctx, struct ex_context *ex_context)
{
    if (ex_context->class == &g_clean_exit) {
        ctx->env->code = ex_context->retcode;
        goto exception_stop;
    }

    if (ex_context->class == &g_keyboard_interrupt || ex_context->class == &g_runtime_error) {
        ctx->env->code = ex_context->retcode;
        goto exception_continue_if_interactive;
    }

    if (ex_context->class != &g_parser_error && ex_context->class != &g_lexer_error)
        errx(2, "received an unknown exception");

    // syntax errors don't have the same return code inside and outside of the REPL
    // $ (
    // > bash: syntax error: unexpected end of file
    // $ echo $?
    // 2
    // $ bash -c '('
    // bash: -c: line 1: syntax error: unexpected end of file
    // $ echo $?

    ctx->env->code = g_cmdopts.src == SHSRC_COMMAND ? 1 : 2;

exception_continue_if_interactive:
    /* continue if the repl is interactive */
    if (context_interactive(ctx))
        return REPL_ACTION_CONTINUE;

exception_stop:
    res->status = REPL_EXCEPTION;
    res->exception_class = ex_context->class;
    return REPL_ACTION_STOP;
}


enum repl_action repl_eof(struct repl_result *res, struct context *ctx)
{
    struct ex_context ex_context;
    struct ex_scope ex_scope = EXCEPTION_SCOPE(&ex_context, NULL);

    /* handle keyboard interupts in initial EOF check */
    if (setjmp(ex_scope.env)) {
        if (ex_context.class != &g_keyboard_interrupt)
            errx(2, "received an unknown exception in EOF check");

        /* propagate the status code from the exception to the repl */
        ctx->env->code = ex_context.retcode;

        /* just stop if not interactive */
        if (!context_interactive(ctx)) {
            res->status = REPL_OK;
            return REPL_ACTION_STOP;
        }

        /* if the stream is interactive, clear the cached EOF and restart parsing */
        cstream_reset(ctx->cs);
        return REPL_ACTION_CONTINUE;
    }

    /* check for EOF with the above context */
    cstream_set_ex_scope(ctx->cs, &ex_scope);
    if (cstream_eof(ctx->cs)) {
        res->status = REPL_OK;
        return REPL_ACTION_STOP;
    }
    cstream_set_ex_scope(ctx->cs, NULL);
    return REPL_ACTION_NONE;
}


void repl(struct repl_result *res, struct context *ctx)
{
    struct ex_context ex_context;
    struct ex_scope ex_scope = EXCEPTION_SCOPE(&ex_context, NULL);

    while (true) {
        ctx->line_start = true;

        /* check for EOF */
        enum repl_action action = repl_eof(res, ctx);
        if (action == REPL_ACTION_CONTINUE)
            continue;
        if (action == REPL_ACTION_STOP)
            break;

         /* parse and execute */
        if (setjmp(ex_scope.env)) {
            /* decide whether to stop running the repl */
            if (handle_repl_exception(res, ctx, &ex_context) == REPL_ACTION_STOP)
                break;

            /* when an interactive non-fatal interupt occurs, cleanup temporary data (tokens, buffers, ...) */
            context_reset(ctx);

            /* restart all over again */
            continue;
        }

        parse(&ctx->ast, ctx->lexer, &ex_scope);

        if (ctx->ast != NULL) {
            /* pretty-print the ast */
            if (g_shopts[SHOPT_AST_PRINT]) {
                FILE *f = fopen("nsh_ast.dot", "w+");
                ast_print(f, ctx->ast);
                fclose(f);
            }

            /* update the shell history */
            history_update(ctx);

            /* execute the parsed AST */
            ctx->env->code = ast_exec(ctx->env, ctx->ast, &ex_scope);

            /* drop the AST reference */
            context_drop_ast(ctx);
        }

        /* reset the error handler */
        cstream_set_ex_scope(ctx->cs, NULL);

        /* prepare the lexer to handle a new line, forgetting all the remaining tokens
         TODO: check whether is it needed, and document the reason */
        lexer_reset(ctx->lexer);
    }

    /* reset the error handler */
    cstream_set_ex_scope(ctx->cs, NULL);
    return;
}
