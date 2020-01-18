#include <stdio.h>

#include "shparse/ast.h"
#include "shexec/environment.h"

int negate_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_negate *negate = (struct shast_negate *)ast;
    return !ast_exec(env, negate->child, cont);
}

int bool_op_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    switch (bool_op->type) {
    case BOOL_AND: {
        int left = ast_exec(env, bool_op->left, cont);
        if (left)
            return left;
        return ast_exec(env, bool_op->right, cont);
    }
    case BOOL_OR:
        if (!ast_exec(env, bool_op->left, cont))
            return 0;
        return ast_exec(env, bool_op->right, cont);
    }
}
