#pragma once

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
int block_exec(s_env *env, struct ast *ast);
