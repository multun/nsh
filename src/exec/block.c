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


static void assignment_exec(struct environment *env, struct shast_assignment *assign, struct ex_scope *ex_scope)
{
    char *name = strdup(assign->name);
    char *value = expand_nosplit(&assign->line_info, assign->value, EXP_FLAGS_ASSIGNMENT, env, ex_scope);
    environment_var_assign(env, name, &sh_string_create(value)->base, false);
}


int block_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    int rc;
    struct shast_block *block = (struct shast_block *)ast;

    // perform variable assignments
    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        assignment_exec(env, assign_vect_get(&block->assigns, i), ex_scope);

    // perform redirections
    struct redir_undo_stack undo_stack = UNDO_STACK_INIT;
    for (size_t i = 0; i < redir_vect_size(&block->redirs); i++)
    {
        struct redir_undo cur_undo = { .count = 0 };
        // do the redirection
        if ((rc = redirection_exec(redir_vect_get(&block->redirs, i), &cur_undo)))
        {
            redir_undo_stack_cancel(&undo_stack);
            return rc;
        }

        // push planned undo operation onto the stack **with alloca**
        for (int undo_i = 0; undo_i < cur_undo.count; undo_i++)
            redir_undo_stack_push(&undo_stack, &cur_undo, undo_i);
    }

    struct ex_scope sub_ex_scope = EXCEPTION_SCOPE(ex_scope->context, ex_scope);
    if (setjmp(sub_ex_scope.env)) {
        // on exceptions, undo redirections and re-raise
        redir_undo_stack_cancel(&undo_stack);
        shraise(ex_scope, NULL);
    }

    // run the command block and undo redirections
    rc = ast_exec(env, block->command, &sub_ex_scope);
    redir_undo_stack_cancel(&undo_stack);
    return rc;
}


int list_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_list *list = (struct shast_list *)ast;
    int res = 0;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        res = ast_exec(env, shast_vect_get(&list->commands, i), ex_scope);
    return res;
}
