#pragma once

#include "ast/ast.h"


/**
** \brief replresents an ast linked list.
*/
typedef struct ast_list
{
  s_ast *ast;
  struct ast_list *next;
} s_ast_list;


/**
** \brief append a new ast to the list
*/
s_ast_list *ast_list_append(s_ast_list *list, s_ast *ast);


/**
** \brief free the list an the ast.
*/
void ast_list_free(s_ast_list *list);
