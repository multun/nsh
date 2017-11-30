#include <stdio.h>

#include "ast/ast.h"


void while_print(FILE *f, s_ast *ast)
{
  s_awhile *awhile = &ast->data.ast_while;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"WHILE\"];\n", id);

  ast_print_rec(f, awhile->condition);
  void *id_cond = awhile->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);

  ast_print_rec(f, awhile->actions);
  void *id_do = awhile->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}
