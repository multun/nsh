#pragma once

#include "ast/ast.h"

/**
** \brief represents an ast linked list.
*/
struct ast_list
{
    struct ast *ast; /**< the current ast */
    struct ast_list *next; /**< the next ast */
};

/**
** \brief append a new ast to the list
*/
struct ast_list *ast_list_append(struct ast_list *list, struct ast *ast);

/**
** \brief free the list an the ast.
*/
void ast_list_free(struct ast_list *list);
