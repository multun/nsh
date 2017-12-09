#pragma once

#include "ast/ast_list.h"
#include "io/cstream.h"
#include "utils/error.h"
#include "shexec/environment.h"


typedef struct context
{
  s_env *env;
  s_ast_list *ast_list;
  bool line_start;
  s_ast *ast;
} s_context;


typedef int (*f_stream_consumer)(s_cstream *cs, s_errcont *errcont,
                                 s_context *context);

int repl(f_stream_consumer consumer);
void context_init(s_context *cont);
void context_destroy(s_context *cont);
