#include <stdio.h>
#include <err.h>

#include "shparse/ast.h"
#include "shexec/environment.h"
#include "shexec/execution.h"

int block_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    int rc;
    struct shast_block *block = (struct shast_block *)ast;

    // perform variable assignments
    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        assignment_exec(env, assign_vect_get(&block->assigns, i), cont);

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


    struct keeper keeper = KEEPER(cont->keeper);
    if (setjmp(keeper.env)) {
        // on exceptions, undo redirections and re-raise
        redir_undo_stack_cancel(&undo_stack);
        shraise(cont, NULL);
    } else {
        // run the command block and undo redirections
        rc = ast_exec(env, block->command, &ERRCONT(cont->errman, &keeper));
        redir_undo_stack_cancel(&undo_stack);
        return rc;
    }
}
