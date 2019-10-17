#pragma once

#include "ast/ast.h"
#include "utils/error.h"

/**
** \brief represents a linked list of ast
*/
struct alist
{
    struct ast *action; /**< the current ast */
    struct alist *next; /**< the next ast */
};

#define ALIST(Action, Next) ((struct alist){(Action), (Next)})

#define AST_ALIST(Action, Next) AST(SHNODE_LIST, list, ALIST(Action, Next))

/**
** \brief print in dot format the representation of a list node
*/
void list_print(FILE *f, struct ast *ast);

/**
** \brief exec a list node
*/
int list_exec(struct environment*env, struct ast *ast, struct errcont *cont);

/**
** \brief free a list node
*/
void list_free(struct ast *ast);
