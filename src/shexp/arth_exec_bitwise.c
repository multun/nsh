#include "shexp/arth.h"

#include <err.h>


int arth_exec_bor(s_arth_ast *ast, s_arthcont *cont)
{
  return arth_exec(ast->left, cont) | arth_exec(ast->right, cont);
}


int arth_exec_band(s_arth_ast *ast, s_arthcont *cont)
{
  return arth_exec(ast->left, cont) & arth_exec(ast->right, cont);
}


int arth_exec_bnot(s_arth_ast *ast, s_arthcont *cont)
{
  return ~arth_exec(ast->left, cont);
}


int arth_exec_xor(s_arth_ast *ast, s_arthcont *cont)
{
  return arth_exec(ast->left, cont) ^ arth_exec(ast->right, cont);
}
