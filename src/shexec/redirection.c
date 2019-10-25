#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shparse/ast.h"
#include "utils/macros.h"
#include "shexec/redirection_utils.h"
#include "shexec/execution.h"


static struct redir_undo_op *redir_undo_reserve(struct redir_undo *undo)
{
    struct redir_undo_op *res = &undo->ops[undo->count++];
    assert(undo->count <= MAX_REDIR_OPS);
    return res;
}

static void redir_undo_plan_close(struct redir_undo *undo, int to_close)
{
    struct redir_undo_op *op = redir_undo_reserve(undo);
    op->type = REDIR_CLOSE;
    op->data.to_close = to_close;
}

static void redir_undo_plan_move(struct redir_undo *undo, int src, int dst)
{
    struct redir_undo_op *op = redir_undo_reserve(undo);
    op->type = REDIR_MOVE;
    op->data.move.src = src;
    op->data.move.dst = dst;
}

static int redir_great(struct shast_redirection *redir,
                       struct redir_undo *undo)
{
    int stdout_copy = fd_move_away(STDOUT_FILENO);

    int fd_file = open(redir->right, O_CREAT | O_WRONLY | O_TRUNC, 0664);
    if (fd_file < 0) {
        warnx("%s: Permission denied\n", redir->right);
        return 1;
    }

    redir_undo_plan_close(undo, fd_file);
    redir_undo_plan_move(undo, stdout_copy, STDOUT_FILENO);
    return 0;
}

static int redir_dgreat(struct shast_redirection *redir,
                        struct redir_undo *undo)
{
    int stdout_copy = fd_move_away(STDOUT_FILENO);

    int fd_file = open(redir->right, O_CREAT | O_WRONLY | O_APPEND, 0664);
    if (fd_file < 0) {
        warnx("%s: Permission denied\n", redir->right);
        return 1;
    }

    redir_undo_plan_close(undo, fd_file);
    redir_undo_plan_move(undo, stdout_copy, STDOUT_FILENO);
    return 0;
}

static int redir_less(struct shast_redirection *redir,
                      struct redir_undo *undo)
{
    int copy = fd_move_away(STDIN_FILENO);

    int fd_file = open(redir->right, O_RDONLY, 0664);
    if (fd_file < 0) {
        warn("cannot open \"%s\"", redir->right);
        return 1;
    }

    redir_undo_plan_close(undo, fd_file);
    redir_undo_plan_move(undo, copy, STDIN_FILENO);
    return 0;
}

static int redir_lessand(struct shast_redirection *redir,
                         struct redir_undo *undo)
{
    if (redir->left == -1)
        redir->left = 0;

    if (!strcmp("-", redir->right))
        if (close(redir->left) < 0)
            errx(1, "redir_lessand: Failed closing %d", redir->left);

    int digit = atoi(redir->right);
    if (dup2(digit, redir->left) < 0) {
        warnx("%s: Bad file descriptor", redir->right);
        return 1;
    }

    redir_undo_plan_close(undo, redir->left);
    return 0;
}

static int redir_greatand(struct shast_redirection *redir,
                          struct redir_undo *undo)
{
    if (redir->left == -1)
        redir->left = 1;
    if (!strcmp("-", redir->right))
        if (close(redir->left) < 0)
            errx(1, "redir_greatand: Failed closing %d", redir->left);

    int save = fd_copy(redir->left);
    int digit = atoi(redir->right);
    if (dup2(digit, redir->left) < 0) {
        warnx("%s: Bad file descriptor", redir->right);
        return 1;
    }

    redir_undo_plan_move(undo, save, redir->left);
    redir_undo_plan_close(undo, save);
    return 0;
}

static int redir_lessgreat(struct shast_redirection *redir,
                           struct redir_undo *undo)
{
    if (redir->left == -1)
        redir->left = 0;
    int fd = open(redir->right, O_CREAT | O_RDWR | O_TRUNC, 0664);
    if (fd < 0) {
        warnx("%s: Permission denied\n", redir->right);
        return 1;
    }

    fd_move(fd, redir->left);
    redir_undo_plan_close(undo, redir->left);
    return 0;
}

static const struct redir_meta
{
    const char *repr;
    int (*func)(struct shast_redirection *redir, struct redir_undo *undo);
} g_redir_list[] =
{
#define REDIRECTIONS_LIST(EName, Repr, Func) {Repr, Func},
    REDIRECTIONS_APPLY(REDIRECTIONS_LIST)
};

static void redirection_print(FILE *f, struct shast_redirection *redir)
{
    void *id = redir;

    if (redir->type >= ARR_SIZE(g_redir_list))
        abort();

    const char *redir_name = g_redir_list[redir->type].repr;

    fprintf(f, "\"%p\" [label=\"%d %s %s\"];\n", id, redir->left, redir_name,
            redir->right);
}

void redir_vect_print(FILE *f, struct redir_vect *vect, void *id)
{
    for (size_t i = 0; i < redir_vect_size(vect); i++)
    {
        struct shast_redirection *redir = redir_vect_get(vect, i);
        redirection_print(f, redir);
        fprintf(f, "\"%p\" -> \"%p\";\n", id, (void*)redir);
    }
}

int redirection_exec(struct shast_redirection *redir, struct redir_undo *undo)
{
    if (redir->type >= ARR_SIZE(g_redir_list))
        abort();

    const struct redir_meta *rmeta = &g_redir_list[redir->type];
    if (rmeta->func == NULL)
        abort();
    return rmeta->func(redir, undo);
}

int redirection_op_cancel(struct redir_undo_op *undo_op)
{
    switch (undo_op->type)
    {
    case REDIR_MOVE:
        fd_move(undo_op->data.move.src, undo_op->data.move.dst);
        return 0;
    case REDIR_CLOSE:
        if (close(undo_op->data.to_close) < 0)
            errx(1, "Failed closing file descriptor");
        return 0;
    }
    abort();
}
