#include <stdio.h>
#include <string.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"


struct for_data {
    struct shast_for *for_node;
    int rc;
};

static void for_expansion_callback(void *data, char *var_value, struct environment *env, struct errcont *cont)
{
    struct for_data *for_data = data;
    struct shast_for *for_node = for_data->for_node;

    /* assign the loop variable */
    char *var_name = strdup(shword_buf(for_node->var));
    environment_var_assign(env, var_name, var_value, false);

    struct errcont sub_errcont = ERRCONT(cont->errman, cont);
    if (setjmp(sub_errcont.env)) {
        /* handle continues by returning from the callback */
        if (cont->errman->class == &g_ex_continue)
            return;

        /* reraise any other exception */
        shraise(cont, NULL);
    }

    /* execute the ast */
    for_data->rc = ast_exec(env, for_node->body, &sub_errcont);
}

int for_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_for *for_node = (struct shast_for *)ast;
    struct for_data for_data = {
        .for_node = for_node,
        .rc = 0,
    };

    env->depth++;

    struct expansion_callback for_callback = {
        .func = for_expansion_callback,
        .data = &for_data,
    };

    struct errcont sub_errcont = ERRCONT(cont->errman, cont);
    if (setjmp(sub_errcont.env)) {
        if (cont->errman->class != &g_ex_break)
            goto reraise;

        assert(env->break_count > 0);
        env->break_count--;
        if (env->break_count != 0)
            goto reraise;
    } else {
        expand_wordlist_callback(&for_callback, &for_node->collection, 0, env, &sub_errcont);
    }

    env->depth--;
    return for_data.rc;

reraise:
    env->depth--;
    shraise(cont, NULL);
}
