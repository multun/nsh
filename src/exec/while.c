#include <stdio.h>

#include "shexec/ast_exec.h"
#include "shexec/break.h"
#include "shexec/environment.h"

int while_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    struct ex_context *ex_context = ex_scope->context;
    struct ex_scope sub_ex_scope = EXCEPTION_SCOPE(ex_context, ex_scope);

    env->depth++;

    if (setjmp(sub_ex_scope.env)) {
        if (ex_context->class == &g_ex_break) {
            /* the break builtin should ensure no impossible break is emitted */
            assert(env->break_count);

            env->break_count--;
            if (env->break_count != 0)
                goto reraise;
            goto exit_while;
        } else if (ex_context->class == &g_ex_continue) {
            /* do nothing */
        } else {
            /* forward all other exception */
            goto reraise;
        }
    }

    volatile int rc = 0;
    while ((ast_exec(env, while_node->condition, &sub_ex_scope) == 0)
           != while_node->is_until)
        rc = ast_exec(env, while_node->body, &sub_ex_scope);

exit_while:
    env->depth--;
    return rc;

reraise:
    env->depth--;
    shraise(ex_scope, NULL);
}
