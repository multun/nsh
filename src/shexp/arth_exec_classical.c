#include "shexp/arth.h"

#include <err.h>


int arth_exec_plus(s_arth_ast *ast)
{
  return arth_exec(ast->left) + arth_exec(ast->right);
}


int arth_exec_minus(s_arth_ast *ast)
{
  if (!ast->right)
    return -arth_exec(ast->left);

  return arth_exec(ast->left) - arth_exec(ast->right);
}


int arth_exec_div(s_arth_ast *ast)
{
  int right = arth_exec(ast->right);
  if (!right)
  {
    warnx("division by 0");
    // TODO better error handling
    return 0;
  }
  return arth_exec(ast->left) / right;
}


int arth_exec_pow(s_arth_ast *ast)
{
  int res = 1;
  int mult = arth_exec(ast->left);
  int exp = arth_exec(ast->right);
  if (exp < 0)
  {
    warnx("exponent less than 0");
    // TODO better error handling
    return 0;
  }
  for (int i = 0; i < exp; i++)
  {
    int newres = res * mult;
    if (mult && newres / mult != res)
      return 0;
    res = newres;
  }
  return res;
}


int arth_exec_time(s_arth_ast *ast)
{
  return arth_exec(ast->left) * arth_exec(ast->right);
}
