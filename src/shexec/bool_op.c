#include <stdio.h>

#include "ast/ast.h"
#include "shexec/environment.h"


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


int bool_op_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_abool_op *abool = &ast->data.ast_bool_op;
  if (abool->type == BOOL_AND)
  {
    int left = ast_exec(env, abool->left, cont);
    if (left)
      return left;
    return ast_exec(env, abool->right, cont);
  }
  if (abool->type == BOOL_OR)
  {
    if (!ast_exec(env, abool->left, cont))
      return 0;
    return ast_exec(env, abool->right, cont);
  }
  else
    return !ast_exec(env, abool->left, cont);
}


void bool_op_free(struct ast *ast)
{
  if (!ast)
    return;
  ast_free(ast->data.ast_bool_op.left);
  ast_free(ast->data.ast_bool_op.right);
  free(ast);
}
