#include "shexp/arth.h"

#include <err.h>


int arth_exec_not(s_arth_ast *ast, s_arthcont *cont)
{
  return !arth_exec(ast->left, cont);
}


int arth_exec_or(s_arth_ast *ast, s_arthcont *cont)
{
  if (arth_exec(ast->left, cont))
    return 1;
  return !!arth_exec(ast->right, cont);
}


int arth_exec_and(s_arth_ast *ast, s_arthcont *cont)
{
  if (!arth_exec(ast->left, cont))
    return 0;
  return !!arth_exec(ast->right, cont);
}
