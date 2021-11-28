#include <stdio.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>


nsh_err_t negate_exec(struct environment *env, struct shast *ast)
{
    struct shast_negate *negate = (struct shast_negate *)ast;
    nsh_err_t err;

    if ((err = ast_exec(env, negate->child)))
        return err;

    env->code = !env->code;
    return NSH_OK;
}


nsh_err_t bool_op_exec(struct environment *env, struct shast *ast)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    nsh_err_t err;

    if ((err = ast_exec(env, bool_op->left)))
        return err;

    if (bool_op->type == BOOL_AND && env->code)
        return NSH_OK;
    if (bool_op->type == BOOL_OR && !env->code)
        return NSH_OK;

    return ast_exec(env, bool_op->right);
}
