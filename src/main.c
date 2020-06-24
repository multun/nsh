#include "cli/cmdopts.h"
#include "repl/repl.h"

#include <locale.h>

/**
** \mainpage Introduction
**
** nsh could be divided into seven main components:
**    - utils, a general toolbox
**    - cli, the command line parsing toolkit
**    - io, the base IO abstractions
**    - shlex, the shell lexer
**    - shparse, the shell parser
**    - shexp, the expression expansion toolkit
**    - shexec, the shell AST evaluation module
**    - repl, the main program loop
**
**  You may find some more detailed explanations for these components
** by clicking the links above.
**
**  Here is a basic overview of what appends:
**   - the \ref cmdopts_parse "cli module" parses the arguments,
**     stores the options and shopts
**   - the io layer \ref cstream_dispatch_init "creates a stream" based
**     on the arguments
**   - a lexer \ref lexer_create "is configured" to call the just configured
**     io layer
**   - the \ref repl "main loop" will call \ref parse "a parser", which will
**     \ref lexer_peek "call the lexer", which will call the io layer, and
**     hopefuly produce an AST.
**   - the \ref repl "main loop" \ref ast_exec "hands the ast" to the shexec
**      module, which will in turn call \ref expand "shexp" when necessary
**   - the operation is repeated as long as the stream
**     \ref cstream_eof "isn't exhausted"
**
**  In order to ensure all allocated chunks are freed,
** \ref shraise "an exception mechanism" has been implemented.
** When an error or a planned exit occurs, a chained sequence of longjmp calls
** goes up the stack and hopefuly frees all allocated data.
**
*/

int main(int argc, char *argv[])
{
    int rc;

    /* parse the arguments */
    int cmdstart = cmdopts_parse(argc, argv);
    if (cmdstart < 0)
        return CMDOPTS_STATUS(cmdstart);
    struct arg_context arg_cont = ARG_CONTEXT(argc, cmdstart, argv);

    /* load the configured locale */
    setlocale(LC_ALL, "");

    struct context cont;

    /* initialize IO */
    struct cstream *cs;
    if ((rc = cstream_dispatch_init(&cont, &cs, &arg_cont)))
        goto err_cstream;

    /* initialize the context the repl will work with */
    if (context_init(&rc, &cont, cs, &arg_cont))
        goto err_context;

    /* run the repl */
    struct repl_result repl_res;
    repl(&repl_res, &cont);
    rc = repl_status(&cont);

    /* cleanup */
    context_destroy(&cont);
err_context:
    cstream_destroy(cs);
    free(cs);
err_cstream:
    return rc;
}
