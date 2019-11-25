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

static void for_expansion_callback(struct expansion_state *exp_state, void *data)
{
    struct errcont *cont = exp_state->errcont;
    struct environment *env = exp_state->env;

    struct for_data *for_data = data;
    struct shast_for *for_node = for_data->for_node;

    // copy the current expansion buffer
    char *var_value = strdup(expansion_result_data(&exp_state->result));

    // assign the loop variable
    char *var_name = strdup(shword_buf(for_node->var));
    environment_var_assign(exp_state->env, var_name, var_value, false);

    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    if (setjmp(keeper.env)) {
        // if the exception isn't a continue, or if it is for an outer loop,
        // re-raise the exception
        if (cont->errman->class != &g_ex_continue
            || --env->break_count != 0)
            shraise(cont, NULL);
    } else {
        // execute the ast
        for_data->rc = ast_exec(exp_state->env, for_node->body, &ncont);
    }
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

    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    if (setjmp(keeper.env)) {
        if ((cont->errman->class != &g_ex_break)
            || --env->break_count) {
            env->depth--;
            shraise(cont, NULL);
        }
    }
    else
        expand_wordlist_callback(&for_callback, &for_node->collection, env, &ncont);

    env->depth--;
    return for_data.rc;
}
