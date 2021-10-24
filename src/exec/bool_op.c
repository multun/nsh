#include <stdio.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>

int negate_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_negate *negate = (struct shast_negate *)ast;
    return !ast_exec(env, negate->child, ex_scope);
}

int bool_op_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    switch (bool_op->type) {
    case BOOL_AND: {
        int left = ast_exec(env, bool_op->left, ex_scope);
        if (left)
            return left;
        return ast_exec(env, bool_op->right, ex_scope);
    }
    case BOOL_OR:
        if (!ast_exec(env, bool_op->left, ex_scope))
            return 0;
        return ast_exec(env, bool_op->right, ex_scope);
    default:
        abort();
    }
}
