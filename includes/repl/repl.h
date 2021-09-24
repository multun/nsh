#pragma once

#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/error.h"
#include "shparse/ast.h"
#include "shexec/clean_exit.h"

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
struct arg_context
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
** \brief parse command line options
** \return if negative, the parsing failed and the expected
**   return code is -(retcode + 1). otherwise, the index of
**   the first non-option argument is returned
*/
int cmdopts_parse(struct arg_context *res, int argc, char *argv[]);


/**
** \brief describes the current context of the read eval loop
*/
struct context
{
    // the runtime environment, such as functions and variables
    struct environment *env;

    // whether we should display the ps1 instead of the ps2
    // it may feel awkward to store this here, but it wouldn't
    // make much sense either to store it inside the stream structure.
    bool line_start;

    struct evect line_buffer;

    // the currently processed ast
    struct shast *ast;

    // the stream the context works on
    struct cstream *cs;

    // the lexer the loops pulls data from
    struct lexer *lexer;

    // the history file, which may be NULL in case none should be opened
    FILE *history;
};

static inline int repl_status(const struct context *ctx)
{
    return ctx->env->code;
}

/**
** \brief checks if the context should be treated as interactive.
**   Interactive contexts restart on keyboard interupts and append to history.
** \param ctx the runtime context
*/
static inline bool context_interactive(struct context *ctx)
{
    return ctx->cs->interactive && !ctx->env->forked;
}

/**
** \brief drop the refence to the AST, if the context holds one
** \param ctx the runtime context
*/
void context_drop_ast(struct context *ctx);


/**
** \brief reset temporary state, and get the context ready for a new repl round
** \param ctx the runtime context
*/
void context_reset(struct context *ctx);

struct repl_result
{
    enum repl_status
    {
        REPL_OK = 0,
        REPL_EXCEPTION,
    } status;

    /* the class of the exception that stopped the loop, if any */
    const struct ex_class *exception_class;
};

static inline bool repl_called_exit(const struct repl_result *repl_res)
{
    return repl_res->status == REPL_EXCEPTION && repl_res->exception_class == &g_clean_exit;
}


/**
** \brief runs shell command from an already setup context
** \param ctx a runtime context
*/
void repl(struct repl_result *res, struct context *ctx);

/**
** \brief initializes a context from command line arguments
** \details this routine also loads rc files, which may fork and exit, thus
**   explaining why this function may require the program to exit
** \param rc the expected return code
** \param cont the context to initialize
** \param cs the cstream to read from during dotfiles evaluation
** \param arg_cont the arguments to read from
** \returns whether the program should exit
*/
bool context_init(int *rc, struct context *cont, struct cstream *cs, struct arg_context *arg_cont);
void context_from_env(struct context *cont, struct cstream *cs, struct environment *env);

/**
** \brief destroys an exiting context and all the ressources allocated
**   by its init twin
*/
void context_destroy(struct context *cont);
