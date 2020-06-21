#include "io/keyboard_interrupt.h"
#include "cli/cmdopts.h"
#include "cli/shopt.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/error.h"

#include <err.h>


// returns whether to continue
static bool handle_repl_exception(struct errman *eman, struct context *ctx)
{
    if (eman->class == &g_clean_exit) {
        ctx->env->code = eman->retcode;
        return false;
    }

    if (eman->class == &g_keyboard_interrupt) {
        ctx->env->code = eman->retcode;
        return true;
    }

    if (eman->class != &g_parser_error && eman->class != &g_lexer_error)
        errx(2, "received an unknown exception");
    // syntax errors don't have the same return code inside and outside
    // of the REPL

    // $ (
    // > bash: syntax error: unexpected end of file
    // $ echo $?
    // 2
    // $ bash -c '('
    // bash: -c: line 1: syntax error: unexpected end of file
    // $ echo $?

    ctx->env->code = g_cmdopts.src == SHSRC_COMMAND ? 1 : 2;
    // stop if the repl isn't interactive
    return ctx->cs->interactive;
}

bool repl(struct context *ctx)
{
    struct errman eman = ERRMAN;
    struct keeper keeper = KEEPER(NULL);
    struct errcont errcont = ERRCONT(&eman, &keeper);

    while (true) {
        ctx->line_start = true;

        /* handle keyboard interupts in initial EOF check */
        if (setjmp(keeper.env)) {
            if (eman.class == &g_keyboard_interrupt) {
                ctx->env->code = eman.retcode;
                continue;
            }
            errx(2, "received an unknown exception in EOF check");
        }

        /* check for EOF with the above context */
        cstream_set_errcont(ctx->cs, &errcont);
        if (cstream_eof(ctx->cs))
            return false;
        cstream_set_errcont(ctx->cs, NULL);

         /* parse and execute */
        if (setjmp(keeper.env)) {
            /* decide whether to stop running the repl */
            if (!handle_repl_exception(&eman, ctx)) {
                cstream_set_errcont(ctx->cs, NULL);
                return true;
            }

            /* when an interactive non-fatal interupt occurs, cleanup temporary data (tokens, buffers, ...) */
            context_reset(ctx);

            /* restart all over again */
            continue;
        }

        parse(&ctx->ast, ctx->lexer, &errcont);

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
            ctx->env->code = ast_exec(ctx->env, ctx->ast, &errcont);

            /* drop the AST reference */
            context_drop_ast(ctx);
        }

        /* reset the error handler */
        cstream_set_errcont(ctx->cs, NULL);

        // prepare the lexer to handle a new line
        // forgetting all the remaining tokens
        lexer_reset(ctx->lexer);
    }
}
