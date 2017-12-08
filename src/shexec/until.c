#include <stdio.h>

#include "ast/ast.h"
#include "shexec/environment.h"


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


int until_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_auntil *auntil = &ast->data.ast_until;
  int res = 0;
  while (ast_exec(env, auntil->condition, cont))
    res = ast_exec(env, auntil->actions, cont);
  return res;
}

void until_free(struct ast *ast)
{
  if (!ast)
    return;
  ast_free(ast->data.ast_until.condition);
  ast_free(ast->data.ast_until.actions);
  free(ast);
}
