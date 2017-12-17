#include "shexp/arth.h"
#include "shexec/clean_exit.h"

#include <err.h>


int arth_exec_plus(s_arth_ast *ast, s_arthcont *cont)
{
  return arth_exec(ast->left, cont) + arth_exec(ast->right, cont);
}


int arth_exec_minus(s_arth_ast *ast, s_arthcont *cont)
{
  if (!ast->right)
    return -arth_exec(ast->left, cont);

  return arth_exec(ast->left, cont) - arth_exec(ast->right, cont);
}


int arth_exec_div(s_arth_ast *ast, s_arthcont *cont)
{
  int right = arth_exec(ast->right, cont);
  if (!right)
  {
    warnx("division by 0");
    clean_exit(cont->cont, 1);
  }
  return arth_exec(ast->left, cont) / right;
}


int arth_exec_pow(s_arth_ast *ast, s_arthcont *cont)
{
  int res = 1;
  int mult = arth_exec(ast->left, cont);
  int exp = arth_exec(ast->right, cont);
  if (exp < 0)
  {
    warnx("exponent less than 0");
    clean_exit(cont->cont, 1);
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


int arth_exec_time(s_arth_ast *ast, s_arthcont *cont)
{
  return arth_exec(ast->left, cont) * arth_exec(ast->right, cont);
}
