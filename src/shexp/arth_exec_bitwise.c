#include "shexp/arth.h"

#include <err.h>


int arth_exec_bor(s_arth_ast *ast)
{
  return arth_exec(ast->left) | arth_exec(ast->right);
}


int arth_exec_band(s_arth_ast *ast)
{
  return arth_exec(ast->left) & arth_exec(ast->right);
}


int arth_exec_bnot(s_arth_ast *ast)
{
  return ~arth_exec(ast->left);
}


int arth_exec_xor(s_arth_ast *ast)
{
  return arth_exec(ast->left) ^ arth_exec(ast->right);
}
