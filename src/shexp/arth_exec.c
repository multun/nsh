#include "shexp/arth.h"

#include <err.h>


static int (*arth_exec_utils[])(s_arth_ast *ast) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_EXEC_UTILS)
};


int arth_exec(s_arth_ast *ast)
{
  if (!ast)
    return 0;
  if (ast->type == ARTH_WORD)
    return ast->value;
  return arth_exec_utils[ast->type](ast);
}
