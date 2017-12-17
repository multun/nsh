#include "shexp/arth.h"

#include <err.h>


int arth_exec_not(s_arth_ast *ast)
{
  return !arth_exec(ast->left);
}


int arth_exec_or(s_arth_ast *ast)
{
  if (arth_exec(ast->left))
    return 1;
  return !!arth_exec(ast->right);
}


int arth_exec_and(s_arth_ast *ast)
{
  if (!arth_exec(ast->left))
    return 0;
  return !!arth_exec(ast->right);
}
