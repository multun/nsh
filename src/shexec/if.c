#include <stdio.h>

#include "ast/ast.h"


void if_print(FILE *f, s_ast *node)
{
  s_aif *aif = &node->data.ast_if;
  void *id = node;
  fprintf(f, "\"%p\" [label=\"IF\"];\n", id);
  ast_print_rec(f, aif->condition);
  void *id_cond = aif->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);
  ast_print_rec(f, aif->success);
  void *id_succ = aif->success;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"THEN\"];\n", id, id_succ);
  if (aif->failure)
  {
    ast_print_rec(f, aif->failure);
    void *id_fail = aif->failure;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"ELSE\"];\n", id, id_fail);
  }
}
