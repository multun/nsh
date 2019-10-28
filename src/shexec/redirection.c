#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shparse/ast.h"
#include "utils/macros.h"
#include "shexec/execution.h"
#include "utils/strutils.h"

static int fd_copy(int fd)
{
    int copy = dup(fd);
    if (copy == -1)
        return copy;

    // TODO: save fd params with GETFD, see strace bash
    if (fcntl(copy, F_SETFD, FD_CLOEXEC) < 0)
        err(1, "fd_save: fcntl failed");
    return copy;
}

static int fd_move_away(int fd)
{
    int copy = fd_copy(fd);
    if (copy == -1)
        return copy;

    if (close(fd))
        err(1, "fd_move_away: close failed");
    return copy;
}

static int fd_move_close(int src, int dst)
{
    if (src == dst)
        return 0;

    if (dup2(src, dst) == -1)
    {
        warn("fd_move_close: dup2 failed");
        return 1;
    }

    if (close(src) == -1)
    {
        warn("fd_move_close: close failed");
        return 1;
    }
    return 0;
}

static int fd_open_into(int dst, const char *path, int flags, mode_t mode)
{
    int fd_file = open(path, flags, mode);
    if (fd_file == -1)
    {
        warn("open('%s') failed", path);
        return -1;
    }

    fd_move_close(fd_file, dst);
    return 0;
}


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

static void redir_undo_plan_restore(struct redir_undo *undo, int src, int dst)
{
    // if the copy is -1, plan a close instead
    if (src == -1)
    {
        redir_undo_plan_close(undo, src);
        return;
    }

    struct redir_undo_op *op = redir_undo_reserve(undo);
    op->type = REDIR_RESTORE;
    op->data.move.src = src;
    op->data.move.dst = dst;
}

static int redir_simple(struct shast_redirection *redir,
                        struct redir_undo *undo,
                        int default_src,
                        int open_flags)
{
    /* choose what file descriptor to redirect */
    int src = redir->left;
    if (src == -1)
        src = default_src;

    /* make a copy of it */
    int src_copy = fd_move_away(src);

    /* open into the choosen file descriptor */
    int rc = fd_open_into(src, shword_buf(redir->right), open_flags, 0664);
    if (rc)
        return rc;

    redir_undo_plan_restore(undo, src_copy, src);
    return 0;

}

static int redir_great(struct shast_redirection *redir,
                       struct redir_undo *undo)
{
    return redir_simple(redir, undo, STDOUT_FILENO, O_CREAT | O_WRONLY | O_TRUNC);
}

static int redir_dgreat(struct shast_redirection *redir,
                        struct redir_undo *undo)
{
    return redir_simple(redir, undo, STDOUT_FILENO, O_CREAT | O_WRONLY | O_APPEND);
}

static int redir_less(struct shast_redirection *redir,
                      struct redir_undo *undo)
{
    return redir_simple(redir, undo, STDIN_FILENO, O_RDONLY);
}

static int redir_lessgreat(struct shast_redirection *redir,
                           struct redir_undo *undo)
{
    return redir_simple(redir, undo, STDIN_FILENO, O_CREAT | O_RDWR | O_TRUNC);
}

// [n]>&-
static int redir_close(struct redir_undo *undo, int fd)
{
    int copy = fd_copy(fd);
    if (copy == -1)
    {
        warn("close failed");
        return 1;
    }
    if (close(fd) == -1)
    {
        warn("close failed");
        return 1;
    }
    redir_undo_plan_restore(undo, copy, fd);
    return 0;
}

// [n]>&fd
static int redir_dup(struct shast_redirection *redir,
                     struct redir_undo *undo,
                     int default_fd)
{
    int left = redir->left;
    if (left == -1)
        left = default_fd;

    if (strcmp(shword_buf(redir->right), "-") == 0)
        return redir_close(undo, left);

    int right;
    if (parse_int(shword_buf(redir->right), &right))
    {
        warn("couldn't parse redirection");
        return 1;
    }

    int left_copy = fd_copy(left);
    if (dup2(right, left) == -1)
    {
        warnx("%s: Bad file descriptor", shword_buf(redir->right));
        return 1;
    }

    redir_undo_plan_restore(undo, left_copy, left);
    return 0;
}

static int redir_lessand(struct shast_redirection *redir,
                         struct redir_undo *undo)
{
    return redir_dup(redir, undo, STDIN_FILENO);
}

static int redir_greatand(struct shast_redirection *redir,
                          struct redir_undo *undo)
{
    return redir_dup(redir, undo, STDOUT_FILENO);
}

static const struct redir_meta
{
    const char *repr;
    int (*func)(struct shast_redirection *redir, struct redir_undo *undo);
} g_redir_list[] =
{
#define REDIRECTIONS_LIST(EName, Repr, Func) [EName] = {Repr, Func},
    REDIRECTIONS_APPLY(REDIRECTIONS_LIST)
#undef REDIRECTIONS_LIST
};

static void redirection_print(FILE *f, struct shast_redirection *redir)
{
    void *id = redir;

    if (redir->type >= ARR_SIZE(g_redir_list))
        abort();

    const char *redir_name = g_redir_list[redir->type].repr;

    fprintf(f, "\"%p\" [label=\"%d %s %s\"];\n", id, redir->left, redir_name,
            shword_buf(redir->right));
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
    case REDIR_RESTORE:
        return fd_move_close(undo_op->data.move.src, undo_op->data.move.dst);
    case REDIR_CLOSE:
        if (close(undo_op->data.to_close) == -1)
        {
            warn("failed closing file descriptor in redirection cleanup");
            return 1;
        }
        return 0;
    }
    abort();
}
