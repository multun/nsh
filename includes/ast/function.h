#pragma once

#include "utils/error.h"
#include "wordlist.h"

/**
** \brief represents a function node
*/
struct afunction
{
    struct wordlist *name; /**< the function name */
    struct ast *value; /**< the function body */
};

#define AFUNCTION(Name, Value) ((struct afunction){(Name), (Value)})

#define AST_AFUNCTION(Name, Value)                                                       \
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
int function_exec(struct environment*env, struct ast *ast, struct errcont *cont);
