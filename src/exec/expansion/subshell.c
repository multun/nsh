#include <nsh_exec/repl.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_utils/evect.h>

#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../execution_error.h"
#include "expansion.h"


static int subshell_child(struct expansion_state *exp_state, const char *str)
{
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", exp_state->line_info);

    struct repl ctx;
    repl_init(&ctx, &cs.base, expansion_state_env(exp_state));

    repl_run(&ctx);
    int rc = repl_status(&ctx);
    repl_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

int expand_subshell(struct expansion_state *exp_state, char *subshell_program)
{
    int err;

    int pipe_fds[2];
    if (pipe(pipe_fds) < 0)
        return expansion_error(exp_state, "pipe() failed: %s", strerror(errno));

    int child_pid = managed_fork(expansion_state_env(exp_state));
    if (child_pid < 0)
        return expansion_error(exp_state, "fork() failed: %s", strerror(errno));

    /* child branch */
    if (child_pid == 0) {
        /* close the read side of the pipe and plug the write side to STDOUT */
        close(pipe_fds[0]);
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);

        /* run the subshell and exit */
        int res = subshell_child(exp_state, subshell_program);
        free(subshell_program);
        return execution_error(expansion_state_env(exp_state), res);
    }

    /* parent branch */
    free(subshell_program);
    close(pipe_fds[1]);

    /* open a buffered reader */
    FILE *child_stream = fdopen(pipe_fds[0], "r");

    /* newlines are counted on the fly: a counter is increased when a newline is
       found. on the next non-newline character, the same number of newlines
       will be injected.

       the accumulated newlines are ignored if there are no more non-newline
       characters. */

    size_t newline_count = 0;
    int cur_char;
    while ((cur_char = fgetc(child_stream)) != EOF) {
        if (cur_char == '\n') {
            newline_count++;
            continue;
        }

        while (newline_count) {
            if ((err = expansion_push_splitable(exp_state, '\n')))
                goto err_expansion;
            newline_count--;
        }
        if ((err = expansion_push_splitable(exp_state, cur_char)))
            goto err_expansion;
    }

    /* Success */
    err = NSH_OK;

err_expansion:
    fclose(child_stream);

    /* cleanup the child process */
    int status;
    waitpid(child_pid, &status, 0);

    return err;
}
