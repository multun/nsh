#pragma once

#include "utils/error.h"
#include "wordlist.h"

/**
** \brief represents an assignment node
*/
struct aassignment
{
    struct wordlist *name; /**< the variable name */
    struct wordlist *value; /**< the value of the variable */
    struct ast *action; /**< the following command */
};

#define AASSIGNMENT(Name, Value, Action)                                                 \
    ((struct aassignment){                                                                    \
        .name = Name,                                                                    \
        .value = Value,                                                                  \
        .action = Action,                                                                \
    })

#define AST_AASSIGNMENT(Name, Value, Action)                                             \
    AST(SHNODE_ASSIGNMENT, assignment, AASSIGNMENT(Name, Value, Action))

/**
** \brief print in dot format the representation of an assignment node
*/
void assignment_print(FILE *f, struct ast *ast);

/**
** \brief make an assignment
*/
void assign_var(struct environment*env, char *name, char *value, bool rm_var);

/**
** \brief exec an assignment node
*/
int assignment_exec(struct environment*env, struct ast *ast, struct ast *cmd, struct errcont *cont);

/**
** \brief free an assignment node
*/
void assignment_free(struct ast *ast);
