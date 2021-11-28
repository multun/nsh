#include <nsh_exec/repl.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_utils/exception.h>
#include <nsh_utils/evect.h>

#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../error_compat.h"
#include "expansion.h"


static int subshell_child(struct expansion_state *exp_state, const char *str)
{
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", exp_state->line_info);

    struct repl ctx;
    repl_init(&ctx, &cs.base, expansion_state_env(exp_state));

    struct repl_result repl_res;
    repl_run(&repl_res, &ctx);
    int rc = repl_status(&ctx);
    repl_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

struct subshell_state
{
    FILE *child_stream;
    int child_pid;
    struct exception_catcher *parent_scope;
};

static inline void subshell_state_cleanup(volatile struct subshell_state *state,
                                          struct expansion_state *exp_state)
{
    fclose(state->child_stream);

    /* cleanup the child process */
    int status;
    waitpid(state->child_pid, &status, 0);

    /* reset the exception handler */
    expansion_state_set_catcher(exp_state, state->parent_scope);
}

void expand_subshell(struct expansion_state *exp_state, char *subshell_program)
{
    volatile struct subshell_state state;
    int pipe_fds[2];
    if (pipe(pipe_fds) < 0) {
        expansion_warning(exp_state, "pipe() failed: %s", strerror(errno));
        nsh_err_t err = execution_error(expansion_state_env(exp_state), 1);
        raise_from_error(expansion_state_catcher(exp_state), err);
    }

    state.child_pid = managed_fork(expansion_state_env(exp_state));
    if (state.child_pid < 0) {
        expansion_warning(exp_state, "fork() failed: %s", strerror(errno));
        nsh_err_t err = execution_error(expansion_state_env(exp_state), 1);
        raise_from_error(expansion_state_catcher(exp_state), err);
    }

    /* child branch */
    if (state.child_pid == 0) {
        /* close the read side of the pipe and plug the write side to STDOUT */
        close(pipe_fds[0]);
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);

        /* run the subshell and exit */
        int res = subshell_child(exp_state, subshell_program);
        free(subshell_program);
        nsh_err_t err = execution_error(expansion_state_env(exp_state), res);
        raise_from_error(expansion_state_catcher(exp_state), err);
    }

    /* parent branch */
    free(subshell_program);
    close(pipe_fds[1]);

    /* open a buffered reader */
    state.child_stream = fdopen(pipe_fds[0], "r");

    /* Most of the expansion routines don't need to have exception handlers,
       as the expansion state can hold everything that needs to be cleaned up.
       It doesn't quite fit here, so the current exception handler for the expansion
       is saved, and restored upon function return. */
    state.parent_scope = expansion_state_catcher(exp_state);
    struct exception_catcher sub_catcher =
        EXCEPTION_CATCHER(state.parent_scope->context, state.parent_scope);
    expansion_state_set_catcher(exp_state, &sub_catcher);

    if (setjmp(sub_catcher.env)) {
        subshell_state_cleanup(&state, exp_state);
        shreraise(state.parent_scope);
    }

    /* newlines are counted on the fly: a counter is increased when a newline is
       found. on the next non-newline character, the same number of newlines
       will be injected.

       the accumulated newlines are ignored if there are no more non-newline
       characters. */

    size_t newline_count = 0;
    int cur_char;
    while ((cur_char = fgetc(state.child_stream)) != EOF) {
        if (cur_char == '\n') {
            newline_count++;
            continue;
        }

        while (newline_count) {
            expansion_push_splitable(exp_state, '\n');
            newline_count--;
        }
        expansion_push_splitable(exp_state, cur_char);
    }

    subshell_state_cleanup(&state, exp_state);
}
