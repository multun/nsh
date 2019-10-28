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
    struct context ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = exp_state->env;

    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", exp_state->line_info);

    ctx.cs = &cs.base;
    repl(&ctx);
    int rc = ctx.env->code;
    ctx.env = NULL; // avoid double free
    context_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

static void subshell_parent(struct expansion_state *exp_state, int cfd)
{
    FILE *creader = fdopen(cfd, "r");

    int cur_char;
    while ((cur_char = fgetc(creader)) != EOF)
        evect_push(&exp_state->vec, cur_char);

    // remove trailing newlines
    size_t size;
    while ((size = evect_size(&exp_state->vec))) {
        if (evect_data(&exp_state->vec)[size - 1] != '\n')
            break;
        exp_state->vec.size--;
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
        clean_exit(exp_state->errcont, res);
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
