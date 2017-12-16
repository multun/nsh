#include "shexp/arth.h"


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
