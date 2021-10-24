#include <stdio.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>

int if_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    int cond = ast_exec(env, if_node->condition, ex_scope);
    if (!cond)
        return ast_exec(env, if_node->branch_true, ex_scope);
    else if (if_node->branch_false)
        return ast_exec(env, if_node->branch_false, ex_scope);
    return 0;
}
