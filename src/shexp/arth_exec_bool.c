#include "shexp/arth.h"

#include <err.h>


int arth_exec_not(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  return !arth_exec(ast->left, env, cont);
}


int arth_exec_or(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  if (arth_exec(ast->left, env, cont))
    return 1;
  return !!arth_exec(ast->right, env, cont);
}


int arth_exec_and(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  if (!arth_exec(ast->left, env, cont))
    return 0;
  return !!arth_exec(ast->right, env, cont);
}
