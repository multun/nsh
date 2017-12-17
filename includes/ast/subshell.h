#pragma once

#include "utils/error.h"
#include "wordlist.h"


/**
** \brief represents a subshell
*/
typedef struct asubshell
{
  struct ast *action; /**< the subshell command */
} s_asubshell;


#define ASUBSHELL(Name, Value, Action)                                        \
  ((s_aassignment)                                                            \
  {                                                                           \
    .action = Action,                                                         \
  })

#define AST_ASUBSHELL(Name, Value, Action)                                    \
  AST(SHNODE_SUBSHELL, asubshell, ASUBSHELL(Name, Value, Action))


/**
** \brief print in dot format the representation of a subshell node
*/
void subshell_print(FILE *f, struct ast *ast);


/**
** \brief exec a subshell node
*/
int subshell_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free a subshell node
*/
void subshell_free(struct ast *ast);
