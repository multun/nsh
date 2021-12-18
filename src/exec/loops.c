#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/expansion.h>
#include <nsh_utils/alloc.h>
#include <stdio.h>
#include <string.h>


static bool stops_loop(struct environment *env, nsh_err_t err)
{
    if (err != NSH_CONTINUE_INTERUPT && err != NSH_BREAK_INTERUPT)
        return false;

    /* the break builtin should ensure no impossible break is emitted */
    assert(env->break_count);
    env->break_count--;
    return env->break_count == 0;
}


static nsh_err_t for_expansion_callback(void *data, char *var_value,
                                        struct environment *env)
{
    nsh_err_t err;
    struct shast_for *for_node = data;

    /* assign the loop variable */
    char *var_name = strdup(shword_buf(for_node->var));
    environment_var_assign_cstring(env, var_name, var_value, false);

    /* execute the ast */
    err = ast_exec(env, for_node->body);

    /* handle continues by returning successfuly from the callback */
    if (err == NSH_CONTINUE_INTERUPT)
        return NSH_OK;

    /* Forward other errors / interupts up */
    return err;
}

nsh_err_t for_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_for *for_node = (struct shast_for *)ast;

    /* If there are no items, the exit status shall be zero. */
    env->code = 0;

    env->depth++;

    struct expansion_callback for_callback = {
        .func = for_expansion_callback,
        .data = for_node,
    };

    /* expand_wordlist_callback calls the loop body for each expanded argument */
    err = expand_wordlist_callback(&for_callback, &for_node->collection, env, 0);
    if (err && !stops_loop(env, err))
        goto err_callback;

    /* continue is directly handled by for_expansion_callback */
    assert(err == NSH_BREAK_INTERUPT || err == NSH_OK);

    /* Success */
    err = NSH_OK;

err_callback:
    env->depth--;
    return err;
}


nsh_err_t while_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_while *while_node = (struct shast_while *)ast;

    env->depth++;

    int exit_status = 0;
    while (true) {
        /* Execute the condition */
        err = ast_exec(env, while_node->condition);
        if (err && !stops_loop(env, err))
            goto err_cond;
        if (err == NSH_BREAK_INTERUPT)
            break;
        if (err == NSH_CONTINUE_INTERUPT)
            continue;

        /* Check the status of the condition */
        if ((env->code == 0) == while_node->is_until)
            break;

        /* Execute the body */
        err = ast_exec(env, while_node->body);
        exit_status = env->code;
        if (err && !stops_loop(env, err))
            goto err_body;
        if (err == NSH_BREAK_INTERUPT)
            break;
    }

    /* Success */
    err = NSH_OK;

err_cond:
err_body:
    if (err != NSH_EXIT_INTERUPT)
        env->code = exit_status;
    env->depth--;
    return err;
}
