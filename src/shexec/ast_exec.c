#include "shexec/environment.h"
#include "ast/ast.h"

static int (*ast_exec_utils[])(struct environment *env, struct ast *ast, struct errcont *cont) = {
    AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)};

int ast_exec(struct environment *env, struct ast *ast, struct errcont *cont)
{
    if (ast)
        env->code = ast_exec_utils[ast->type](env, ast, cont);
    else
        env->code = 0;
    return env->code;
}

static void (*ast_free_utils[])(struct ast *ast) = {AST_TYPE_APPLY(DECLARE_AST_FREE_UTILS)};

void ast_free(struct ast *ast)
{
    if (!ast)
        return;
    ast_free_utils[ast->type](ast);
}
