#include <stdio.h>

#include "ast/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"

void while_print(FILE *f, s_ast *ast)
{
    s_awhile *awhile = &ast->data.ast_while;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"WHILE\"];\n", id);

    ast_print_rec(f, awhile->condition);
    void *id_cond = awhile->condition;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);

    ast_print_rec(f, awhile->actions);
    void *id_do = awhile->actions;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}

int while_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
    s_awhile *awhile = &ast->data.ast_while;
    volatile int res = 0;
    s_keeper keeper = KEEPER(cont->keeper);
    s_errcont ncont = ERRCONT(cont->errman, &keeper);
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
        while (!ast_exec(env, awhile->condition, &ncont))
            res = ast_exec(env, awhile->actions, &ncont);

    env->depth--;
    return res;
}

void while_free(struct ast *ast)
{
    if (!ast)
        return;
    ast_free(ast->data.ast_while.condition);
    ast_free(ast->data.ast_while.actions);
    free(ast);
}
