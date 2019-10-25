#include <stdio.h>

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
        struct redir_undo cur_undo;
        // do the redirection
        if ((rc = redirection_exec(redir_vect_get(&block->redirs, i), &cur_undo)))
            return rc;

        // for each planned undo operation, push it onto the stack with alloca
        for (int undo_i = 0; undo_i < cur_undo.count; undo_i++)
            redir_undo_stack_push(&undo_stack, &cur_undo, undo_i);
    }

    rc = ast_exec(env, block->command, cont);

    // undo the redirections
    for (size_t i = 0; i < undo_stack.size; i++)
        redirection_op_cancel(redir_undo_stack_get(&undo_stack, i));

    return rc;
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
