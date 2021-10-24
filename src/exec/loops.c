#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/expansion.h>
#include <nsh_utils/alloc.h>
#include <stdio.h>
#include <string.h>

#include "break.h"


struct for_data {
    struct shast_for *for_node;
    int rc;
};

static void for_expansion_callback(void *data, char *var_value, struct environment *env, struct exception_catcher *catcher)
{
    struct for_data *for_data = data;
    struct shast_for *for_node = for_data->for_node;

    /* assign the loop variable */
    char *var_name = strdup(shword_buf(for_node->var));
    environment_var_assign_cstring(env, var_name, var_value, false);

    struct exception_catcher sub_catcher = EXCEPTION_CATCHER(catcher->context, catcher);
    if (setjmp(sub_catcher.env)) {
        /* handle continues by returning from the callback */
        if (catcher->context->class == &g_ex_continue)
            return;

        /* reraise any other exception */
        shraise(catcher, NULL);
    }

    /* execute the ast */
    for_data->rc = ast_exec(env, for_node->body, &sub_catcher);
}

int for_exec(struct environment *env, struct shast *ast, struct exception_catcher *catcher)
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

    struct exception_catcher sub_catcher = EXCEPTION_CATCHER(catcher->context, catcher);
    if (setjmp(sub_catcher.env)) {
        if (catcher->context->class != &g_ex_break)
            goto reraise;

        assert(env->break_count > 0);
        env->break_count--;
        if (env->break_count != 0)
            goto reraise;
    } else {
        expand_wordlist_callback(&for_callback, &for_node->collection, 0, env, &sub_catcher);
    }

    env->depth--;
    return for_data.rc;

reraise:
    env->depth--;
    shraise(catcher, NULL);
}


int while_exec(struct environment *env, struct shast *ast, struct exception_catcher *catcher)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    struct exception_context *ex_context = catcher->context;
    struct exception_catcher sub_catcher = EXCEPTION_CATCHER(ex_context, catcher);

    env->depth++;

    if (setjmp(sub_catcher.env)) {
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
    while ((ast_exec(env, while_node->condition, &sub_catcher) == 0)
           != while_node->is_until)
        rc = ast_exec(env, while_node->body, &sub_catcher);

exit_while:
    env->depth--;
    return rc;

reraise:
    env->depth--;
    shraise(catcher, NULL);
}
