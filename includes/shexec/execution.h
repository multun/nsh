#pragma once

#include "shparse/ast.h"

struct redir_undo_op
{
    enum redir_undo_type
    {
        REDIR_MOVE,
        REDIR_CLOSE,
    } type;
    union {
        struct {
            int src;
            int dst;
        } move;
        int to_close;
    } data;
};

#define MAX_REDIR_OPS 2
struct redir_undo
{
    int count;
    struct redir_undo_op ops[MAX_REDIR_OPS];
};

struct redir_undo_stack
{
    size_t size;
    struct redir_undo_op *ops;
};

static inline struct redir_undo_op *redir_undo_stack_get(struct redir_undo_stack *stack, size_t i)
{
    return &stack->ops[-i];
}

#define UNDO_STACK_INIT { .size = 0, .ops = NULL }

static inline void redir_undo_stack_push(struct redir_undo_stack *stack, struct redir_undo *undo, int i, struct redir_undo_op *dst)
{
    struct redir_undo_op *cur_undo_op = &undo->ops[undo->count - i - 1];
    stack->size++;
    stack->ops = dst;
    *stack->ops = *cur_undo_op;
}

#define redir_undo_stack_push(Stack, Undo, I) \
    redir_undo_stack_push(Stack, Undo, I, alloca(sizeof(struct redir_undo_op)))

int redirection_exec(struct shast_redirection *redir, struct redir_undo *undo);
int redirection_op_cancel(struct redir_undo_op *undo_op);
