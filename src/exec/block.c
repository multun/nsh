#include <stdio.h>
#include <err.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/execution.h>
#include <nsh_exec/expansion.h>


/*
** Blocks and lists work together to group commands:
**  - list nodes are used to execute a sequence of commands
**  - blocks are used to perform assignments and redirections wherever needed
**
** For example:
**  - "echo a"              is parsed as   block { echo a }
**  - "echo a > test_file"  is parsed as   block(">test_file") { echo a }
**  - "{ echo a; echo b; }" is parsed as   block { list [ block { echo a }, block { echo b } ] }
**  - "{ echo a; }"         is parsed as   block { block { echo a } }
*/


static nsh_err_t assignment_exec(struct environment *env, struct shast_assignment *assign)
{
    nsh_err_t err;
    char *name = strdup(assign->name);
    char *value;

    if ((err = expand_nosplit(&value, &assign->line_info, assign->value, env,
                              EXP_FLAGS_ASSIGNMENT)))
        return err;

    environment_var_assign(env, name, &sh_string_create(value)->base, false);
    return NSH_OK;
}


nsh_err_t block_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_block *block = (struct shast_block *)ast;

    // perform variable assignments
    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        if ((err = assignment_exec(env, assign_vect_get(&block->assigns, i))))
            return err;

    // perform redirections
    struct redir_undo_stack undo_stack = UNDO_STACK_INIT;
    for (size_t i = 0; i < redir_vect_size(&block->redirs); i++) {
        struct redir_undo cur_undo = {.count = 0};
        // do the redirection
        if ((err = redirection_exec(redir_vect_get(&block->redirs, i), &cur_undo))) {
            redir_undo_stack_cancel(&undo_stack);
            return err;
        }

        // push planned undo operation onto the stack **with alloca**
        for (int undo_i = 0; undo_i < cur_undo.count; undo_i++)
            redir_undo_stack_push(&undo_stack, &cur_undo, undo_i);
    }

    // run the command block and undo redirections
    if (block->command)
        err = ast_exec(env, block->command);
    else {
        env->code = 0;
        err = NSH_OK;
    }
    redir_undo_stack_cancel(&undo_stack);
    return err;
}


nsh_err_t list_exec(struct environment *env, struct shast *ast)
{
    struct shast_list *list = (struct shast_list *)ast;
    nsh_err_t err;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        if ((err = ast_exec(env, shast_vect_get(&list->commands, i))))
            return err;
    return NSH_OK;
}
