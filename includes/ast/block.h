#pragma once

#include "ast/ast.h"
#include "utils/error.h"


typedef struct ablock
{
  struct ast *redir;
  struct ast *def;
  struct ast *cmd;
} s_ablock;

#define ABLOCK(Redir, Def, Cmd)   \
  ((s_ablock)                     \
  {                               \
    .redir = Redir,               \
    .def = Def,                   \
    .cmd = Cmd,                   \
  })



void block_print(FILE *f, struct ast *node);
int block_exec(s_env *env, struct ast *ast, s_errcont *cont);
void block_free(struct ast *ast);
