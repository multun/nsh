#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"

/**
** \brief represents an if node
*/
struct aif
{
    struct ast *condition; /**< the if condition */
    struct ast *success; /**< the command to execute in case of success*/
    struct ast *failure; /**< the command to execute in case of failure*/
};

#define AIF(Condition, Success, Failure) ((struct aif){(Condition), (Success), (Failure)})

#define AST_AIF(Condition, Success, Failure)                                             \
    AST(SHNODE_IF, if, AIF(Condition, Success, Failure))

/**
** \brief print in dot format the representation of an if node
*/
void if_print(FILE *f, struct ast *node);

/**
** \brief exec an if node
*/
int if_exec(struct environment*env, struct ast *ast, struct errcont *cont);

/**
** \brief free an if node
*/
void if_free(struct ast *ast);
