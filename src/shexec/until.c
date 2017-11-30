#include <stdio.h>

#include "ast/ast.h"


void until_print(FILE *f, s_ast *ast)
{
  s_auntil *auntil = &ast->data.ast_until;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"UNTIL\"];\n", id);

  ast_print_rec(f, auntil->condition);
  void *id_cond = auntil->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);

  ast_print_rec(f, auntil->actions);
  void *id_do = auntil->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}
