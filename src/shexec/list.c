#include <stdio.h>

#include "ast/ast.h"


void list_print(FILE *f, s_ast *ast)
{
  s_alist *alist = &ast->data.ast_list;

  void *id = ast;
  fprintf(f, "\"%p\" [label=\"LIST\"];\n", id);

  while (alist)
  {
    ast_print_rec(f, alist->action);
    void *id_cur = alist->action;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_cur);
    alist = alist->next;
  }
}


int list_exec(s_env *env, s_ast *ast)
{
  s_alist *alist = &ast->data.ast_list;
  while (alist->next)
  {
    ast_exec(env, alist->action);
    alist = alist->next;
  }
  return ast_exec(env, alist->action);
}
