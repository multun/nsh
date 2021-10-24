#include <nsh_exec/ast_exec.h>


#define AST_EXEC_UTILS(EnumName, Name) \
    [EnumName] = Name ## _exec,

static int (*ast_exec_utils[])(struct environment *env,
                               struct shast *ast,
                               struct ex_scope *ex_scope) =
{
    AST_TYPE_APPLY(AST_EXEC_UTILS)
};

int ast_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    if (ast) {
        //printf("executing an asynchronous node\n");
        env->code = ast_exec_utils[ast->type](env, ast, ex_scope);
    } else
        env->code = 0;
    return env->code;
}
 