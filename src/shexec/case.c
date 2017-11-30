#include <stdio.h>

#include "ast/ast.h"


void case_print(FILE *f, s_ast *ast)
{
  s_acase *acase = &ast->data.ast_case;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"CASE\"];\n", id);
  s_acase_node *node = acase->nodes;

  while (node)
  {
    ast_print_rec(f, node->action);
    void *id_next = node->action;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"%s\"];\n", id, id_next,
            node->pattern->str);
    node = node->next;
  }
}
