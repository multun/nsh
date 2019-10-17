#pragma once

#include "utils/error.h"
#include "wordlist.h"

/**
** \brief represents a subshell
*/
struct asubshell
{
    struct ast *action; /**< the subshell command */
};

#define ASUBSHELL(Name, Value, Action)                                                   \
    ((struct aassignment){                                                                    \
        .action = Action,                                                                \
    })

#define AST_ASUBSHELL(Name, Value, Action)                                               \
    AST(SHNODE_SUBSHELL, asubshell, ASUBSHELL(Name, Value, Action))

/**
** \brief print in dot format the representation of a subshell node
*/
void subshell_print(FILE *f, struct ast *ast);

/**
** \brief exec a subshell node
*/
int subshell_exec(struct environment *env, struct ast *ast, struct errcont *cont);

/**
** \brief free a subshell node
*/
void subshell_free(struct ast *ast);
