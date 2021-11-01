#pragma once

#include <nsh_parse/ast.h>

struct redir_undo_op
{
    enum redir_undo_type
    {
        REDIR_RESTORE,
        REDIR_CLOSE,
    } type;
    union
    {
        struct
        {
            int src;
            int dst;
        } move;
        int to_close;
    } data;
    struct redir_undo_op *next;
};

#define MAX_REDIR_OPS 2
struct redir_undo
{
    int count;
    struct redir_undo_op ops[MAX_REDIR_OPS];
};

struct redir_undo_stack
{
    struct redir_undo_op *last_op;
};

#define UNDO_STACK_INIT                                                                  \
    {                                                                                    \
        .last_op = NULL                                                                  \
    }

static inline void redir_undo_stack_push(struct redir_undo_stack *stack,
                                         struct redir_undo *undo, int i,
                                         struct redir_undo_op *dst)
{
    struct redir_undo_op *cur_undo_op = &undo->ops[undo->count - i - 1];
    *dst = *cur_undo_op;
    dst->next = stack->last_op;
    stack->last_op = dst;
}

#define redir_undo_stack_push(Stack, Undo, I)                                            \
    redir_undo_stack_push(Stack, Undo, I, alloca(sizeof(struct redir_undo_op)))

int redirection_exec(struct shast_redirection *redir, struct redir_undo *undo);
int redirection_op_cancel(struct redir_undo_op *undo_op);

static inline void redir_undo_stack_cancel(struct redir_undo_stack *undo_stack)
{
    for (struct redir_undo_op *op = undo_stack->last_op; op; op = op->next)
        redirection_op_cancel(op);
}
