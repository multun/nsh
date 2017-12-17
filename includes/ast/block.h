#pragma once

#include "ast/ast.h"
#include "utils/error.h"


/**
** \brief represents a block node
*/
typedef struct ablock
{
  struct ast *redir; /**< the redirection node */
  struct ast *def; /**< the assignment node */
  struct ast *cmd; /**< the command node */
} s_ablock;

#define ABLOCK(Redir, Def, Cmd)   \
  ((s_ablock)                     \
  {                               \
    .redir = Redir,               \
    .def = Def,                   \
    .cmd = Cmd,                   \
  })


/**
** \brief print in dot format the representation of a block node
*/
void block_print(FILE *f, struct ast *node);


/**
** \brief exec a block node
*/
int block_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free a block node
*/
void block_free(struct ast *ast);
