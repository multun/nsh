#include "shexec/repl.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "shexec/managed_fork.h"
#include "shexec/runtime_error.h"
#include "shexec/expansion.h"
#include "utils/error.h"
#include "utils/evect.h"

#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


static int subshell_child(struct expansion_state *exp_state, const char *str)
{
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", exp_state->line_info);

    struct context ctx;
    context_from_env(&ctx, &cs.base, expansion_state_env(exp_state));

    struct repl_result repl_res;
    repl(&repl_res, &ctx);
    int rc = repl_status(&ctx);
    context_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

struct subshell_state
{
    FILE *child_stream;
    int child_pid;
    struct ex_scope *parent_scope;
};

static inline void subshell_state_cleanup(volatile struct subshell_state *state, struct expansion_state *exp_state)
{
    fclose(state->child_stream);

    /* cleanup the child process */
    int status;
    waitpid(state->child_pid, &status, 0);

    /* reset the exception handler */
    expansion_state_set_ex_scope(exp_state, state->parent_scope);
}

void expand_subshell(struct expansion_state *exp_state, char *subshell_content)
{
    volatile struct subshell_state state;
    int pipe_fds[2];
    if (pipe(pipe_fds) < 0) {
        expansion_warning(exp_state, "pipe() failed: %s", strerror(errno));
        runtime_error(expansion_state_ex_scope(exp_state), 1);
    }

    state.child_pid = managed_fork(expansion_state_env(exp_state));
    if (state.child_pid < 0) {
        expansion_warning(exp_state, "fork() failed: %s", strerror(errno));
        runtime_error(expansion_state_ex_scope(exp_state), 1);
    }

    /* child branch */
    if (state.child_pid == 0) {
        /* close the read side of the pipe and plug the write side to STDOUT */
        close(pipe_fds[0]);
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);

        /* run the subshell and exit */
        int res = subshell_child(exp_state, subshell_content);
        free(subshell_content);
        clean_exit(expansion_state_ex_scope(exp_state), res);
    }

    /* parent branch */
    free(subshell_content);
    close(pipe_fds[1]);

    /* open a buffered reader */
    state.child_stream = fdopen(pipe_fds[0], "r");

    /* Most of the expansion routines don't need to have exception handlers,
       as the expansion state can hold everything that needs to be cleaned up.
       It doesn't quite fit here, so the current exception handler for the expansion
       is saved, and restored upon function return. */
    state.parent_scope = expansion_state_ex_scope(exp_state);
    struct ex_scope sub_ex_scope = EXCEPTION_SCOPE(state.parent_scope->context, state.parent_scope);
    expansion_state_set_ex_scope(exp_state, &sub_ex_scope);

    if (setjmp(sub_ex_scope.env)) {
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
