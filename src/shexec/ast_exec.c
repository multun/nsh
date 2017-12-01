#include "ast/ast.h"


static int (*ast_exec_utils[])(s_ast *ast) =
{
  AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)
};


int ast_exec(s_ast *ast)
{
  if (ast)
    return ast_exec_utils[ast->type](ast);
  return 0;
}
