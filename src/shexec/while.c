#include <stdio.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"

int while_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    volatile int res = 0;
    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    env->depth++;

    volatile bool local_continue = true;
    if (setjmp(keeper.env)) {
        // the break builtin ensures no impossible break is emitted
        if (cont->errman->class != &g_lbreak || --env->break_count) {
            env->depth--;
            shraise(cont, NULL);
        }
        local_continue = env->break_continue;
    }

    if (local_continue)
        while ((ast_exec(env, while_node->condition, &ncont) == 0)
               != while_node->is_until)
            res = ast_exec(env, while_node->body, &ncont);

    env->depth--;
    return res;
}
