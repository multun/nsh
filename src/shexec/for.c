#include <stdio.h>
#include <string.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

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

int for_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_for *for_node = (struct shast_for *)ast;

    volatile int ret = 0;
    volatile bool local_continue = true;
    volatile size_t i = 0;

    env->depth++;
    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    if (setjmp(keeper.env))
        for_exception_handler(&local_continue, cont, env, &i);

    struct wordlist *wl = &for_node->collection;
    if (local_continue)
        for (; i < wordlist_size(wl); i++) {
            char *var_value = expand_nosplit(&ast->line_info, shword_buf(wordlist_get(wl, i)), env, cont);
            environment_var_assign(env, strdup(shword_buf(for_node->var)), var_value, false);
            ret = ast_exec(env, for_node->body, &ncont);
        }

    env->depth--;
    return ret;
}
