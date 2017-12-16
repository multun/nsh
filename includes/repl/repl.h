#pragma once

#include "ast/ast_list.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


typedef struct arg_context
{
  int progname_ind;
  int argc;
  int argc_base;
  char **argv;
} s_arg_context;


#define ARG_CONTEXT(Argc, ArgcBase, Argv)       \
  (s_arg_context)                               \
  {                                             \
    .progname_ind = 0,                          \
    .argc = (Argc),                             \
    .argc_base = (ArgcBase),                    \
    .argv = (Argv)                              \
  }


typedef struct context
{
  s_env *env;
  bool line_start;
  s_ast *ast;
  s_cstream *cs;
  s_lexer *lexer;
  FILE *history;
} s_context;


/**
** \brief runs shell command from an already setup context
*/
bool repl(s_context *ctx);



/**
** \brief initializes a context from command line arguments
*/
bool context_init(int *rc, s_context *cont, s_arg_context *arg_cont);
void context_destroy(s_context *cont);
