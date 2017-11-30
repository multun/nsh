#include <stdio.h>

#include "ast/ast.h"


void bool_op_print(FILE *f, s_ast *ast)
{
  s_abool_op *abool = &ast->data.ast_bool_op;
  void *id = ast;
  if (abool->type == BOOL_OR)
    fprintf(f, "\"%p\" [label=\"OR\"];\n", id);
  else if (abool->type == BOOL_AND)
    fprintf(f, "\"%p\" [label=\"AND\"];\n", id);
  else
    fprintf(f, "\"%p\" [label=\"NOT\"];\n", id);

  ast_print_rec(f, abool->left);
  void *id_left = abool->left;
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

  if (abool->type != BOOL_NOT)
  {
    ast_print_rec(f, abool->right);
    void *id_right = abool->right;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
  }
}
