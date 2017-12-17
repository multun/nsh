#pragma once

#include "utils/error.h"
#include "wordlist.h"


/**
** \brief represents a function node
*/
typedef struct afunction
{
  s_wordlist *name; /**< the function name */
  struct ast *value; /**< the function body */
} s_afunction;


#define AFUNCTION(Name, Value)                                                \
  ((s_afunction)                                                              \
  {                                                                           \
    (Name), (Value)                                                           \
  })

#define AST_AFUNCTION(Name, Value)                                            \
  AST(SHNODE_FUNCTION, function, AFUNCTION(Name, Value)


/**
** \brief print in dot format the representation of a function node
*/
void function_print(FILE *f, struct ast *ast);


/**
** \brief exec a function node
*/
void function_free(struct ast *ast);


/**
** \brief free a function node
*/
int function_exec(s_env *env, struct ast *ast, s_errcont *cont);
