#pragma once

#include "ast/ast_list.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/error.h"

#include <stdbool.h>


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
bool context_init(int *rc, s_context *cont, int argc, char *argv[]);
void context_destroy(s_context *cont);
