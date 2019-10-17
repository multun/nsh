#pragma once

#include "ast/ast.h"

/**
** \brief represents a pipe node
*/
struct apipe
{
    struct ast *left; /**< the left operand */
    struct ast *right; /**< the right operand */
};

#define APIPE(Left, Right) ((struct apipe){(Left), (Right)})

#define AST_PIPE(Left, Right) AST(SHNODE_PIPE, pipe, APIPE(Left, Right))

/**
** \brief print in dot format the representation of a pipe node
*/
void pipe_print(FILE *f, struct ast *ast);

/**
** \brief exec a pipe node
*/
void pipe_free(struct ast *ast);

/**
** \brief free a pipe node
*/
int pipe_exec(struct environment*env, struct ast *ast, struct errcont *cont);
