#include <err.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "shparse/ast.h"
#include "shexec/clean_exit.h"
#include "shexec/managed_fork.h"
#include "shexec/runtime_error.h"
#include "utils/safe_syscalls.h"


/* The way file descriptors are chained in a pipeline looks like that:
**
** +-------------------+ +-------------------------+ +---------------------+
** |    process A      | |        process B        | |      process C      |
** | STDIN   pipe0[in] | |  pipe0[out]   pipe1[in] | | pipe1[out]   STDOUT |
** +-------------------+ +-------------------------+ +---------------------+
**
** The shell prepares file descriptors for each child in a queue.
** The child dups the FDs to stdin and stdout, and discards the rest of the
** queue. The parent closes the FDs it doesn't need anymore after spawning the
** child, and repeats.
*/

#define FD_QUEUE_CAPACITY 4

/* a ring buffer used as a queue */
struct fd_queue
{
    int buffer[FD_QUEUE_CAPACITY];
    size_t size;
    size_t pos;
};


static inline int fd_queue_pop(struct fd_queue *queue)
{
    int res = queue->buffer[queue->pos++];
    if (queue->pos == FD_QUEUE_CAPACITY)
        queue->pos = 0;
    assert(queue->size != 0);
    queue->size--;
    return res;
}


static int fd_queue_pop_close(struct fd_queue *queue)
{
    int next_fd = fd_queue_pop(queue);

    /* don't close enqueued standard FDs */
    if (next_fd == STDIN_FILENO || next_fd == STDOUT_FILENO || next_fd == STDERR_FILENO)
        return 0;

    if (safe_close(next_fd) == 0)
        return 0;

    warnx("fd_queue_pop_close: failed closing %d", next_fd);
    return -1;
}

/* close all the pipe FDs remaining in the queue */
static int fd_queue_flush(struct fd_queue *queue)
{
    while (queue->size)
        if (fd_queue_pop_close(queue) < 0)
            return -1;
    return 0;
}

static inline void fd_queue_push(struct fd_queue *queue, int fd)
{
    size_t offset = (queue->pos + queue->size++) % FD_QUEUE_CAPACITY;
    assert(queue->size <= FD_QUEUE_CAPACITY);
    queue->buffer[offset] = fd;
}


static int pipeline_move_fd(int source, int destination)
{
    if (source == destination)
        return 0;

    if (safe_dup2(source, destination) < 0) {
        warnx("pipeline_move_fd: failed dup of %d to %d", source, destination);
        return -1;
    }

    if (safe_close(source) < 0) {
        warnx("pipeline_move_fd: failed closing %d", source);
        return -1;
    }

    return 0;
}

static int pipeline_setup_child(struct fd_queue *queue)
{
    int in_fd = fd_queue_pop(queue);
    if (pipeline_move_fd(in_fd, STDIN_FILENO) < 0)
        return -1;

    int out_fd = fd_queue_pop(queue);
    if (pipeline_move_fd(out_fd, STDOUT_FILENO) < 0)
        return -1;

    return fd_queue_flush(queue);
}

static int pipeline_enqueue_pipe(struct fd_queue *queue)
{
    int fd[2];
    if (pipe(fd) < 0) {
        warn("pipeline_enqueue_pipe: pipe() failed");
        return -1;
    }

    fd_queue_push(queue, fd[1]);
    fd_queue_push(queue, fd[0]);
    return 0;
}

int pipeline_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    int status = 0;
    struct shast_pipeline *pipe = (struct shast_pipeline *)ast;
    size_t pipeline_size = shast_vect_size(&pipe->children);

    assert(pipeline_size);

    /* prepare an array of children PIDs */
    int *children_pids = xcalloc(sizeof(*children_pids), pipeline_size);
    for (size_t i = 0; i < pipeline_size; i++)
        children_pids[i] = -1;

    /* this is the state of the pipeline */
    struct fd_queue queue = { 0 };

    size_t child_i = 0;
    for (; child_i < pipeline_size; child_i++) {
        /* create the required pipes / child IO configuration*/
        if (child_i == 0)
            fd_queue_push(&queue, STDIN_FILENO);
        if (child_i == pipeline_size - 1)
            fd_queue_push(&queue, STDOUT_FILENO);
        else
            pipeline_enqueue_pipe(&queue);

        int child_pid = managed_fork(env);
        /* error handling */
        if (child_pid == -1) {
            warn("pipe_exec: fork() failed");
            /* copied from bash's */
            status = 254;
            goto runtime_error;
        }

        /* child branch */
        if (child_pid == 0) {
            free(children_pids);
            int res = pipeline_setup_child(&queue);
            if (res == 0) {
                struct shast *ast = shast_vect_get(&pipe->children, child_i);
                res = ast_exec(env, ast, ex_scope);
            }
            clean_exit(ex_scope, res);
            abort();
        }

        /* parent branch */
        children_pids[child_i] = child_pid;

        /* close the input and output FDs of the just spawned process. */
        int rc __unused = 0;
        rc |= fd_queue_pop_close(&queue);
        rc |= fd_queue_pop_close(&queue);
        assert(rc == 0 && "failed to close a just opened pipe");
    }

    /* the above loop is expected to progressively close
       all pipe FDs at just the right time */
    assert(queue.size == 0);

    /* wait for children processes to terminate */
    for (size_t i = 0; i < pipeline_size; i++) {
        int child_status;
        waitpid(children_pids[i], &child_status, 0);
        status = WEXITSTATUS(child_status);
    }

    free(children_pids);
    return status;

runtime_error:
    if (fd_queue_flush(&queue) < 0)
        warnx("pipeline_exec: failed to cleanup pipe FDs on error");

    for (size_t i = 0; i < child_i; i++) {
        int child_status;
        waitpid(children_pids[i], &child_status, 0);
    }
    free(children_pids);

    ex_scope->context->retcode = status;
    shraise(ex_scope, &g_runtime_error);
}
