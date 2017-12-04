#include <stdio.h>

#include "ast/ast.h"


void list_print(FILE *f, s_ast *ast)
{
  s_alist *alist = &ast->data.ast_list;

  while (alist)
  {
    ast_print_rec(f, alist->action);
    if (alist->next)
    {
      void *id = alist->action;
      void *id_next = alist->next->action;
      fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
    }
    alist = alist->next;
  }
}
