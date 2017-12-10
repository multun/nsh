#pragma once

#include "ast/ast_list.h"
#include "io/cstream.h"
#include "io/managed_stream.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/error.h"

typedef struct context
{
  s_env *env;
  s_ast_list *ast_list;
  bool line_start;
  s_ast *ast;
  s_managed_stream ms;
  s_lexer *lexer;
  FILE *history;
} s_context;


typedef int (*f_stream_consumer)(s_cstream *cs, s_errcont *errcont,
                                 s_context *context);

int repl(s_context *ctx);

int context_init(s_context *cont, int argc, char *argv[]);
void context_destroy(s_context *cont);
