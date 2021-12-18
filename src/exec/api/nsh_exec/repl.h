#pragma once

#include <nsh_io/cstream.h>
#include <nsh_exec/environment.h>
#include <nsh_parse/ast.h>
#include <nsh_exec/clean_exit.h>

#include <stdbool.h>

/**
** \brief what kind of source the user provided
** \details by default, read from stdin, which is a file
*/
enum shsrc
{
    SHSRC_FILE = 0,
    SHSRC_COMMAND = 1,
};


/**
** \brief describes the interpreted command line of the program
*/
struct cli_options
{
    int argc;
    char **argv;

    // the index of the program name inside the array.
    // may not be 0 in case -c is used
    int progname_ind;

    // the index of the first positional argument
    int argc_base;

    // was the norc option passed?
    int norc;

    // the source to read data from.
    // the underlying type is enum shsrc, but it's an
    // int to comply with getopt
    enum shsrc src;

    // the command, as passed to -c, or NULL
    const char *command;

    // all shopts
    bool shopts[SHOPT_COUNT];
};


/**
** \brief describes the current context of the read eval loop
*/
struct repl
{
    // the runtime environment, such as functions and variables
    struct environment *env;

    struct evect line_buffer;

    void (*add_history)(const char *command);

    // the currently processed ast
    struct shast *ast;

    // the stream the context works on
    struct cstream *cs;

    // the lexer the loops pulls data from
    struct lexer *lexer;

    // the history file, which may be NULL in case none should be opened
    FILE *history;
};

static inline int repl_status(const struct repl *repl)
{
    return repl->env->code;
}

/**
** \brief checks if the context should be treated as interactive.
**   Interactive contexts restart on keyboard interupts and append to history.
** \param ctx the runtime context
*/
static inline bool repl_is_interactive(struct repl *repl)
{
    return repl->cs->interactive && !repl->env->forked;
}

/**
** \brief drop the refence to the AST, if the context holds one
** \param ctx the runtime context
*/
void repl_drop_ast(struct repl *ctx);


/**
** \brief reset temporary state, and get the context ready for a new repl round
** \param ctx the runtime context
*/
void repl_reset(struct repl *ctx);


/**
** \brief runs shell command from an already setup context
** \param ctx a runtime context
*/
nsh_err_t repl_run(struct repl *ctx);

/**
** \brief destroys an exiting context and all the ressources allocated
**   by its init twin
*/
void repl_destroy(struct repl *cont);


/**
** \brief initializes the repl
*/
void repl_init(struct repl *ctx, struct cstream *cs, struct environment *env);
