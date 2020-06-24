#include <stdio.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"

int while_exec(struct environment *env, struct shast *ast, struct errcont *errcont)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    struct errman *errman = errcont->errman;
    struct errcont sub_errcont = ERRCONT(errman, errcont);

    env->depth++;

    if (setjmp(sub_errcont.env)) {
        if (errman->class == &g_ex_break) {
            /* the break builtin should ensure no impossible break is emitted */
            assert(env->break_count);

            env->break_count--;
            if (env->break_count != 0)
                goto reraise;
            goto exit_while;
        } else if (errman->class == &g_ex_continue) {
            /* do nothing */
        } else {
            /* forward all other exception */
            goto reraise;
        }
    }

    volatile int rc = 0;
    while ((ast_exec(env, while_node->condition, &sub_errcont) == 0)
           != while_node->is_until)
        rc = ast_exec(env, while_node->body, &sub_errcont);

exit_while:
    env->depth--;
    return rc;

reraise:
    env->depth--;
    shraise(errcont, NULL);
}
