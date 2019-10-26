#include <stdio.h>
#include <err.h>

#include "shparse/ast.h"
#include "shexec/environment.h"
#include "shexec/execution.h"

void block_print(FILE *f, struct shast *node)
{
    struct shast_block *block = (struct shast_block *)node;
    void *id = node;
    fprintf(f, "\"%p\" [label=\"BLOCK\"];\n", id);
    redir_vect_print(f, &block->redirs, node);
    assign_vect_print(f, &block->assigns, node);
    if (block->command) {
        ast_print_rec(f, block->command);
        void *id_next = block->command;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"CMD\"];\n", id, id_next);
    }
}

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

void block_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_block *block = (struct shast_block *)ast;
    for (size_t i = 0; i < redir_vect_size(&block->redirs); i++)
        shast_redirection_free(redir_vect_get(&block->redirs, i));
    redir_vect_destroy(&block->redirs);

    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        shast_assignment_free(assign_vect_get(&block->assigns, i));
    assign_vect_destroy(&block->assigns);

    ast_free(block->command);
    free(ast);
}
