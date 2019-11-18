#include <stdio.h>
#include <string.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"


int for_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_for *for_node = (struct shast_for *)ast;

    env->depth++;

    volatile int rc = 0;
    struct wordlist *wl = &for_node->collection;
    for (volatile size_t i = 0; i < wordlist_size(wl); i++) {
        struct keeper keeper = KEEPER(cont->keeper);
        struct errcont ncont = ERRCONT(cont->errman, &keeper);
        if (setjmp(keeper.env)) {
            if ((cont->errman->class != &g_ex_break
                 && cont->errman->class != &g_ex_continue)
                || --env->break_count) {
                env->depth--;
                shraise(cont, NULL);
            }

            if (cont->errman->class == &g_ex_continue)
                continue;
            else
                break;
        }

        char *var_value = expand_nosplit(&ast->line_info, shword_buf(wordlist_get(wl, i)), env, &ncont);
        environment_var_assign(env, strdup(shword_buf(for_node->var)), var_value, false);
        rc = ast_exec(env, for_node->body, &ncont);
    }

    env->depth--;
    return rc;
}
