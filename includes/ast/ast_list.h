#pragma once

#include "ast/ast.h"

/**
** \brief represents an ast linked list.
*/
typedef struct ast_list
{
    s_ast *ast; /**< the current ast */
    struct ast_list *next; /**< the next ast */
} s_ast_list;

/**
** \brief append a new ast to the list
*/
s_ast_list *ast_list_append(s_ast_list *list, s_ast *ast);

/**
** \brief free the list an the ast.
*/
void ast_list_free(s_ast_list *list);
