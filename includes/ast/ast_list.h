#pragma once

#include "ast/ast.h"


typedef struct ast_list
{
  s_ast *ast;
  struct ast_list *next;
} s_ast_list;


s_ast_list *ast_list_append(s_ast_list *list, s_ast *ast);
void ast_list_free(s_ast_list *list);
