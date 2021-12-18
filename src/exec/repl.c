#include <nsh_io/keyboard_interrupt.h>
#include <nsh_exec/ast_exec.h>
#include <nsh_exec/history.h>
#include <nsh_exec/repl.h>
#include <nsh_exec/shopt.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_lex/lexer.h>
#include <nsh_parse/parse.h>
#include <nsh_parse/parser_error.h>
#include <nsh_utils/exception.h>

#include <err.h>

#include "error_compat.h"

enum repl_action
{
    REPL_ACTION_NONE = 0,
    REPL_ACTION_STOP,
    REPL_ACTION_CONTINUE,
};

static enum repl_action repl_handle_err(nsh_err_t err, struct repl *ctx)
{
    switch (err) {
    case NSH_OK:
        return REPL_ACTION_NONE;
    case NSH_IO_ERROR:
    case NSH_LEXER_ERROR:
    case NSH_PARSER_ERROR:
        // with bash, syntax errors don't always yield the same error code
        // bash -c '('; echo $?       -> 1
        // echo '(' | bash; echo $?   -> 2
        // INTERACTIVE ) THEN echo $?'-> 2
        // we just ignore this madness
        ctx->env->code = 2;
        goto exception_continue_if_interactive;
    case NSH_EXECUTION_ERROR:
    case NSH_EXPANSION_ERROR:
    case NSH_KEYBOARD_INTERUPT:
        goto exception_continue_if_interactive;
    case NSH_BREAK_INTERUPT:
    case NSH_CONTINUE_INTERUPT:
        abort();
    case NSH_EXIT_INTERUPT:
        return REPL_ACTION_STOP;
    }

    /* unknown error */
    abort();

exception_continue_if_interactive:
    /* continue if the repl is interactive */
    if (repl_is_interactive(ctx))
        return REPL_ACTION_CONTINUE;
    return REPL_ACTION_STOP;
}


enum repl_action repl_eof(nsh_err_t *err, struct repl *ctx)
{
    int rc;
    if ((rc = cstream_eof(ctx->cs)) < 0) {
        assert(rc == NSH_KEYBOARD_INTERUPT);

        /* if the stream is interactive, restart parsing */
        if (repl_is_interactive(ctx))
            return REPL_ACTION_CONTINUE;

        /* just stop if not interactive */
        *err = rc;
        return REPL_ACTION_STOP;
    }

    /* if there's no EOF, do nothing */
    if (!rc)
        return REPL_ACTION_NONE;

    /* if interactive, print the exit message */
    if (repl_is_interactive(ctx))
        printf("exit\n");
    *err = NSH_OK;
    return REPL_ACTION_STOP;
}


nsh_err_t repl_run(struct repl *ctx)
{
    nsh_err_t err = NSH_OK;
    enum repl_action action;

    for (;; repl_reset(ctx)) {
        /* check for EOF */
        if ((action = repl_eof(&err, ctx)))
            goto handle_repl_action;

        /* parse an AST */
        if ((err = parse(&ctx->ast, ctx->lexer))) {
            if ((action = repl_handle_err(err, ctx)))
                goto handle_repl_action;
        }

        /* skip empty ASTs */
        if (ctx->ast == NULL)
            continue;

        /* pretty-print the ast */
        if (ctx->env->shopts[SHOPT_AST_PRINT]) {
            FILE *f = fopen("nsh_ast.dot", "w+");
            ast_print(f, ctx->ast);
            fclose(f);
        }

        /* update the shell history */
        history_update(ctx);

        /* execute the parsed AST */
        if ((err = ast_exec(ctx->env, ctx->ast))) {
            if ((action = repl_handle_err(err, ctx)))
                goto handle_repl_action;
        }

        /* drop the AST reference */
        repl_drop_ast(ctx);
        continue;

    handle_repl_action:
        if (action == REPL_ACTION_STOP)
            break;
    }
    return err;
}
