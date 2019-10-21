#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

void for_print(FILE *f, struct ast *ast)
{
    struct afor *afor = &ast->data.ast_for;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"FOR %s in", id, afor->var);
    struct wordlist *wl = &afor->collection;
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        if (i > 0)
            fputc(' ', f);
        fprintf(f, "%s", wordlist_get(wl, i));
    }
    fprintf(f, "\"];\n");
    ast_print_rec(f, afor->actions);
    void *id_do = afor->actions;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}

static void for_exception_handler(volatile bool *local_continue, struct errcont *cont,
                                  struct environment *env, size_t volatile *i)
{
    // the break builtin ensures no impossible break is emitted
    if (cont->errman->class != &g_lbreak || --env->break_count) {
        env->depth--;
        shraise(cont, NULL);
    }

    if ((*local_continue = env->break_continue))
        (*i)++;
}

int for_exec(struct environment *env, struct ast *ast, struct errcont *cont)
{
    struct afor *afor = &ast->data.ast_for;

    volatile int ret = 0;
    volatile bool local_continue = true;
    volatile size_t i = 0;

    env->depth++;
    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    if (setjmp(keeper.env))
        for_exception_handler(&local_continue, cont, env, &i);

    struct wordlist *wl = &afor->collection;
    if (local_continue)
        for (; i < wordlist_size(wl); i++) {
            environment_var_assign(env, strdup(afor->var), expand(&ast->line_info, wordlist_get(wl, i), env, cont), false);
            ret = ast_exec(env, afor->actions, &ncont);
        }

    env->depth--;
    return ret;
}

void for_free(struct ast *ast)
{
    if (!ast)
        return;

    free(ast->data.ast_for.var);
    wordlist_destroy(&ast->data.ast_for.collection);
    ast_free(ast->data.ast_for.actions);
    free(ast);
}
