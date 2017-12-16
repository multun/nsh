#include "shexp/arth.h"

#include <err.h>


static int arth_exec_or(s_arth_ast *ast)
{
  if (arth_exec(ast->left))
    return 1;
  return !!arth_exec(ast->right);
}


static int arth_exec_and(s_arth_ast *ast)
{
  if (!arth_exec(ast->left))
    return 0;
  return !!arth_exec(ast->right);
}


static int arth_exec_bor(s_arth_ast *ast)
{
  return arth_exec(ast->left) | arth_exec(ast->right);
}


static int arth_exec_band(s_arth_ast *ast)
{
  return arth_exec(ast->left) & arth_exec(ast->right);
}


static int arth_exec_xor(s_arth_ast *ast)
{
  return arth_exec(ast->left) ^ arth_exec(ast->right);
}


static int arth_exec_plus(s_arth_ast *ast)
{
  return arth_exec(ast->left) + arth_exec(ast->right);
}


static int arth_exec_minus(s_arth_ast *ast)
{
  return arth_exec(ast->left) - arth_exec(ast->right);
}


static int arth_exec_div(s_arth_ast *ast)
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


static int arth_exec_pow(s_arth_ast *ast)
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
    if (mult && newres / mult != newres)
      return 0;
    res = newres;
  }
  return res;
}


static int arth_exec_time(s_arth_ast *ast)
{
  return arth_exec(ast->left) * arth_exec(ast->right);
}


static int (*arth_exec_utils[])(s_arth_ast *ast) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_EXEC_UTILS)
};


int arth_exec(s_arth_ast *ast)
{
  if (ast->type == ARTH_WORD)
    return ast->value;
  return arth_exec_utils[ast->type](ast);
}
