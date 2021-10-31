#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_utils/macros.h>
#include <nsh_exec/execution.h>
#include <nsh_utils/strutils.h>
#include <nsh_utils/logging.h>


/**
** Try to make a copy, or return -1 if the file descriptor doesn't exist
*/
static int fd_try_copy(int fd)
{
    // store the copied fd somewhere inaccessible
    int copy = fcntl(fd, F_DUPFD, 10);
    if (copy == -1)
        return copy;

    nsh_info("made a copy of %d (%d)", fd, copy);

    // TODO: save fd params with GETFD, see strace bash
    if (fcntl(copy, F_SETFD, FD_CLOEXEC) < 0)
        err(1, "fd_save: fcntl failed");
    return copy;
}

static int fd_try_move_away(int fd)
{
    nsh_info("moving away %d", fd);
    int copy = fd_try_copy(fd);
    if (copy == -1)
        return -1;

    nsh_info("closing %d", fd);
    if (close(fd))
        err(1, "fd_try_move_away: close failed");
    return copy;
}

static int fd_move_close(int src, int dst)
{
    nsh_info("moving %d into %d, and closing %d", src, dst, src);
    if (src == dst)
        return 0;

    if (dup2(src, dst) == -1) {
        warn("fd_move_close: dup2 failed");
        return 1;
    }

    if (close(src) == -1) {
        warn("fd_move_close: close failed");
        return 1;
    }
    return 0;
}

static int fd_open_into(int dst, const char *path, int flags, mode_t mode)
{
    int fd_file = open(path, flags, mode);
    if (fd_file == -1) {
        warn("open('%s') failed", path);
        return -1;
    }
    nsh_info("opened %s, got fd %d. Moving to fd %d", path, fd_file, dst);
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
    assert(to_close > 0);
    struct redir_undo_op *op = redir_undo_reserve(undo);
    op->type = REDIR_CLOSE;
    op->data.to_close = to_close;
}

static void redir_undo_plan_restore(struct redir_undo *undo, int src, int dst)
{
    // when we're planning to restore -1,
    // it means there was no file descriptor to restore, and thus that
    // we can instead close dst
    if (src == -1) {
        redir_undo_plan_close(undo, dst);
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
    int src_copy = fd_try_move_away(src);

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
    int copy = fd_try_move_away(fd);
    if (copy == -1) {
        warn("close failed");
        return 1;
    }
    redir_undo_plan_restore(undo, copy, fd);
    return 0;
}


// [n]>&word or [n]<&word (> makes n default to stdout, < to stdin)
// basicaly means fd(n) = fd[word], or dup2(word, n)
static int redir_dup(struct shast_redirection *redir,
                     struct redir_undo *undo,
                     int default_fd)
{
    int left = redir->left;
    if (left == -1)
        left = default_fd;

    // if word evaluates to '-', file descriptor n,
    // or standard output if n is not specified, is closed
    if (strcmp(shword_buf(redir->right), "-") == 0)
        return redir_close(undo, left);

    int right;
    if (parse_int(shword_buf(redir->right), &right)) {
        warn("couldn't parse redirection");
        return 1;
    }

    // make a save of the left file descriptor
    int left_copy = fd_try_copy(left);
    if (left_copy == -1)
        nsh_info("making a copy of %d failed", left);

    // duplicate the right fd into the left one (that's why a save was made)
    nsh_info("duplicating %d into %d", right, left);
    if (dup2(right, left) == -1) {
        // TODO: handle closing left_copy
        warn("failed to dup %d", right);
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

static int redir_unimplemented(struct shast_redirection *redir,
                               struct redir_undo *undo);

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

static int redir_unimplemented(struct shast_redirection *redir,
                               struct redir_undo *undo __unused)
{
    warnx("unimplemented redirection: %s", g_redir_list[redir->type].repr);
    return 1;
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
    nsh_info("undoing redirection:");
    switch (undo_op->type)
    {
    case REDIR_RESTORE:
        return fd_move_close(undo_op->data.move.src, undo_op->data.move.dst);
    case REDIR_CLOSE:
        nsh_info("closing %d", undo_op->data.to_close);
        if (close(undo_op->data.to_close) == -1) {
            warn("failed closing file descriptor in redirection cleanup");
            return 1;
        }
        return 0;
    }
    abort();
}
