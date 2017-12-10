#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"


/**
** \brief represents a for node
*/
typedef struct afor
{
  s_wordlist *var;
  s_wordlist *collection;
  struct ast *actions;
} s_afor;


#define AFOR(Var, Collection, Actions)                            \
  ((s_afor)                                                       \
  {                                                               \
    (Var), (Collection), (Actions)                                \
  })

#define AST_AFOR(Var, Collection, Actions)                        \
  AST(SHNODE_FOR, for, AFOR(Var, Collection, Actions))


/**
** \brief print in dot format the representation of a for node
*/
void for_print(FILE *f, struct ast *ast);


/**
** \brief exec a for node
*/
int for_exec(s_env *env, struct ast *ast, s_errcont *cont);


/**
** \brief free a for node
*/
void for_free(struct ast *ast);
