#include "shparse/ast.h"
#include "shexec/environment.h"

#define AST_EXEC_UTILS(EnumName, Name) \
    [EnumName] = Name ## _exec,

static int (*ast_exec_utils[])(struct environment *env, struct shast *ast, struct errcont *cont) = {
    AST_TYPE_APPLY(AST_EXEC_UTILS)};

int ast_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    if (ast)
        env->code = ast_exec_utils[ast->type](env, ast, cont);
    else
        env->code = 0;
    return env->code;
}

#define AST_FREE_UTILS(EnumName, Name) \
    [EnumName] = Name ## _free,
static void (*ast_free_utils[])(struct shast *ast) = {AST_TYPE_APPLY(AST_FREE_UTILS)};

void ast_free(struct shast *ast)
{
    if (!ast)
        return;
    ast_free_utils[ast->type](ast);
}
