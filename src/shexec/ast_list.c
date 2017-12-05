#include "ast/ast_list.h"
#include "utils/alloc.h"


s_ast_list *ast_list_append(s_ast_list *list, s_ast *ast)
{
  s_ast_list *new = xmalloc(sizeof (s_ast_list));
  new->next = list;
  new->ast = ast;
  return new;
}


void ast_list_free(s_ast_list *list)
{
  while (list)
  {
    ast_free(list->ast);
    s_ast_list *tmp = list->next;
    free(list);
    list = tmp;
  }
}
