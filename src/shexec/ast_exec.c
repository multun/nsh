#include "shexec/environment.h"
#include "ast/ast.h"


static int (*ast_exec_utils[])(s_env *env, s_ast *ast) =
{
  AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)
};


int ast_exec(s_env *env, s_ast *ast)
{
  if (ast)
    return ast_exec_utils[ast->type](env, ast);
  return 0;
}

static void (*ast_free_utils[])(s_ast *ast) =
{
  AST_TYPE_APPLY(DECLARE_AST_FREE_UTILS)
};

void ast_free(s_ast *ast)
{
  if (!ast)
    return;
  ast_free_utils[ast->type](ast);
}
