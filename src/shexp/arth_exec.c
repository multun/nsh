#include "shexp/arth.h"


static int arth_exec_or(s_arth_ast *ast)
{
  if (arth_exec(ast->left))
    return 1;
  return !!arth_exec(ast->right);
}


int arth_exec(s_arth_ast *ast)
{
  if (ast->type == ARTH_OR)
    return arth_exec_or(ast);
  if (ast->type == ARTH_WORD)
    return ast->value;
  return 0;
}
