#pragma once

#include "ast/ast.h"
#include "shexec/environment.h"
#include "utils/error.h"

/**
** \brief represents a while node
*/
typedef struct awhile
{
    struct ast *condition; /**< the loop condition */
    struct ast *actions; /**< the actions to execute into the loop */
} s_awhile;

#define AWHILE(Condition, Actions) ((s_awhile){(Condition), (Actions)})

#define AST_AWHILE(Condition, Actions)                                                   \
    AST(SHNODE_WHILE, while, AWHILE(Condition, Actions))

/**
** \brief print in dot format the representation of a while node
*/
void while_print(FILE *f, struct ast *ast);

/**
** \brief exec a while node
*/
int while_exec(s_env *env, struct ast *ast, s_errcont *cont);

/**
** \brief free a while node
*/
void while_free(struct ast *ast);
