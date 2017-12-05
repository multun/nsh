#include <stdio.h>

#include "ast/ast.h"


void pipe_print(FILE *f, s_ast *ast)
{
  s_apipe *apipe = &ast->data.ast_pipe;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"|\"];\n", id);

  void *id_left = apipe->left;
  ast_print_rec(f, apipe->left);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

  void *id_right = apipe->right;
  ast_print_rec(f, apipe->right);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
}


void pipe_free(struct ast *ast)
{
  if (!ast)
    return;
  ast_free(ast->data.ast_pipe.left);
  ast_free(ast->data.ast_pipe.right);
  free(ast);
}
