#include "cli/cmdopts.h"
#include "cli/shopt.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/error.h"

static void try_read_eval(s_lexer *lex, s_errcont *errcont, s_context *cont)
{
    parse(&cont->ast, lex, errcont);
    if (!cont->ast)
        return;

    if (g_shopts[SHOPT_AST_PRINT]) {
        FILE *f = fopen("42sh_ast.dot", "w+");
        ast_print(f, cont->ast);
        fclose(f);
    }

    history_update(cont);
    cont->env->code = ast_exec(cont->env, cont->ast, errcont);
}

// returns whether to stop the loop
static bool handle_repl_exception(s_errman *eman, s_context *cont)
{
    if (eman->class == &g_clean_exit) {
        cont->env->code = eman->retcode;
        return true;
    }

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
    // don't stop the repl if interactive
    return !cont->cs->interactive;
}

static bool ast_exec_consumer(s_lexer *lex, s_context *cont)
{
    cont->line_start = true;
    cont->ast = NULL;

    s_errman eman = ERRMAN;
    s_keeper keeper = KEEPER(NULL);

    volatile bool stopping = false;
    if (setjmp(keeper.env)) {
        if (handle_repl_exception(&eman, cont))
            stopping = true;
    } else
        try_read_eval(lex, &ERRCONT(&eman, &keeper), cont);

    cont->env->ast_list = ast_list_append(cont->env->ast_list, cont->ast);
    return stopping;
}

bool repl(s_context *ctx)
{
    bool stopping = false;
    while (!stopping && !cstream_eof(ctx->cs)) {
        s_lexer *lex = lexer_create(ctx->cs);
        stopping = ast_exec_consumer(lex, ctx);
        lexer_free(lex);
    }

    return stopping;
}
