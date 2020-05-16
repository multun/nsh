#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/error.h"
#include "utils/evect.h"

#include <err.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int subshell_child(struct expansion_state *exp_state, char *str)
{
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", exp_state->line_info);

    struct context ctx;
    context_from_env(&ctx, &cs.base, expansion_state_env(exp_state));

    repl(&ctx);
    int rc = ctx.env->code;
    context_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

static void subshell_parent(struct expansion_state *exp_state, int cfd)
{
    FILE *creader = fdopen(cfd, "r");

    // newlines are counted on the fly: a counter is increased when a newline is
    // found. on the next non-newline character, the same number of newlines
    // will be injected.

    // the accumulated newlines are ignored if there are no more non-newline
    // characters.

    size_t newline_count = 0;
    int cur_char;
    while ((cur_char = fgetc(creader)) != EOF) {
        if (cur_char == '\n') {
            newline_count++;
            continue;
        }

        while (newline_count) {
            expansion_push(exp_state, '\n');
            newline_count--;
        }
        expansion_push(exp_state, cur_char);
    }

    fclose(creader);
}

// TODO: handle child exit errors
void expand_subshell(struct expansion_state *exp_state, char *buf)
{
    int pfd[2];
    // TODO: handle errors
    if (pipe(pfd) < 0)
        err(1, "pipe() failed");

    int cpid = fork();
    if (cpid < 0)
        err(1, "fork() failed");

    // child branch
    if (cpid == 0) {
        close(pfd[0]);
        dup2(pfd[1], 1);
        int res = subshell_child(exp_state, buf);
        free(buf);
        close(pfd[1]);
        clean_exit(expansion_state_errcont(exp_state), res);
    }
    // parent branch
    else
    {
        free(buf);
        close(pfd[1]);
        subshell_parent(exp_state, pfd[0]);
        close(pfd[0]);
        int status;
        waitpid(cpid, &status, 0);
    }
}
