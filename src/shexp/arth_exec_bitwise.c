#include "shexp/arth.h"

#include <err.h>


int arth_exec_bor(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  return arth_exec(ast->left, env, cont) | arth_exec(ast->right, env, cont);
}


int arth_exec_band(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  return arth_exec(ast->left, env, cont) & arth_exec(ast->right, env, cont);
}


int arth_exec_bnot(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  return ~arth_exec(ast->left, env, cont);
}


int arth_exec_xor(s_arth_ast *ast, s_env *env, s_errcont *cont)
{
  return arth_exec(ast->left, env, cont) ^ arth_exec(ast->right, env, cont);
}
