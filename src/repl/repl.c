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

static void try_read_eval(struct lexer *lex, struct errcont *errcont, struct context *cont)
{
    struct shast *ast = NULL;
    parse(&ast, lex, errcont);
    if (ast == NULL)
        return;

    if (g_shopts[SHOPT_AST_PRINT]) {
        FILE *f = fopen("42sh_ast.dot", "w+");
        ast_print(f, ast);
        fclose(f);
    }

    history_update(cont);
    cont->env->code = ast_exec(cont->env, ast, errcont);
    shast_ref_put(ast);
}

// returns whether to continue
static bool handle_repl_exception(struct errman *eman, struct context *cont)
{
    if (eman->class == &g_clean_exit) {
        cont->env->code = eman->retcode;
        return false;
    }

    if (eman->class == &g_keyboard_interrupt) {
        cont->env->code = eman->retcode;
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

    cont->env->code = g_cmdopts.src == SHSRC_COMMAND ? 1 : 2;
    // stop if the repl isn't interactive
    return cont->cs->interactive;
}

bool repl(struct context *ctx)
{
    struct errman eman = ERRMAN;
    struct keeper keeper = KEEPER(NULL);
    struct errcont errcont = ERRCONT(&eman, &keeper);

    volatile bool running = true;
    do {
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

        /* create a lexer */
        struct lexer *lex = lexer_create(ctx->cs);

         /* parse and execute */
        if (setjmp(keeper.env))
            running = handle_repl_exception(&eman, ctx);
        else
            try_read_eval(lex, &errcont, ctx);


        /* reset the error handler */
        cstream_set_errcont(ctx->cs, NULL);

        /* destroy the lexer */
        lexer_free(lex);
    } while (running);
    return true;
}
