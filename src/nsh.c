#include <nsh_interactive/interactive_repl.h>
#include <nsh_interactive/cli.h>
#include <nsh_exec/repl.h>
#include <nsh_utils/logging.h>
#include <locale.h>


/**
** \mainpage Introduction
**
** nsh could be divided into seven main components:
**    - utils, a general toolbox
**    - io, the base IO abstractions
**    - lex, the shell lexer
**    - parse, the shell parser
**    - exec, the shell AST evaluation module
**    - repl, the main program loop
**
**  You may find some more detailed explanations for these components
** by clicking the links above.
**
**  Here is a basic overview of what appends:
**   - the command line arguments are parsed
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
*/


int main(int argc, char *argv[])
{
    int rc;
    nsh_log_setup_environ();

    nsh_debug("starting up");

    /* parse the arguments */
    struct cli_options options;
    if ((rc = parse_cli_options(&options, argc, argv)) != 0)
        return rc;

    /* load the configured locale */
    setlocale(LC_ALL, "");

    /* initialize the repl and io backend */
    struct repl repl;
    struct cstream *cs;
    if ((rc = interactive_repl_init(&repl, &options, &cs)))
        return rc;

    /* run the repl */
    repl_run(&repl);
    rc = repl_status(&repl);

    /* cleanup */
    repl_destroy(&repl);
    cstream_free(cs);
    nsh_log_teardown();
    return rc;
}
