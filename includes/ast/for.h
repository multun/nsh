#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"

/**
** \brief represents a for node
*/
struct afor
{
    char *var; /**< the variable */
    struct wordlist collection; /**< the list of value */
    struct ast *actions; /**< the action to execute for each value */
};

#define AFOR(Var, Collection, Actions) ((struct afor){(Var), (Collection), (Actions)})
static inline void afor_init(struct afor *node)
{
    wordlist_init(&node->collection);
}

#define AST_AFOR(Var, Collection, Actions)                                               \
  AST(SHNODE_FOR, for, AFOR(Var, Collection, Actions))

/**
** \brief print in dot format the representation of a for node
*/
void for_print(FILE *f, struct ast *ast);

/**
** \brief exec a for node
*/
int for_exec(struct environment*env, struct ast *ast, struct errcont *cont);

/**
** \brief free a for node
*/
void for_free(struct ast *ast);
