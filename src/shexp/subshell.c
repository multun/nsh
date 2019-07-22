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

static int subshell_child(struct environment *env, char *str)
{
    s_context ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = env;

    struct cstream_string cs = { 0 };
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<subshell>", NULL);

    ctx.cs = &cs.base;
    repl(&ctx);
    int rc = ctx.env->code;
    ctx.env = NULL; // avoid double free
    context_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

static void subshell_parent(int cfd, struct evect *res)
{
    FILE *creader = fdopen(cfd, "r");
    int cur_char;

    // TODO: optimize
    while ((cur_char = fgetc(creader)) != EOF)
        evect_push(res, cur_char);

    // remove trailing newlines
    while (res->size) {
        if (res->data[res->size - 1] != '\n')
            break;
        res->size--;
    }

    fclose(creader);
}

// TODO: handle child exit errors
void expand_subshell(struct errcont *cont, char *buf, struct environment *env,
                     struct evect *vec)
{
    int pfd[2];
    // TODO: handle errors
    if (pipe(pfd) < 0)
        err(1, "pipe() failed");

    int cpid = fork();
    if (cpid < 0)
        err(1, "fork() failed");

    if (!cpid) {
        close(pfd[0]);
        dup2(pfd[1], 1);
        int res = subshell_child(env, buf);
        free(buf);
        close(pfd[1]);
        clean_exit(cont, res);
    }

    free(buf);
    close(pfd[1]);
    subshell_parent(pfd[0], vec);
    close(pfd[0]);
    int status;
    waitpid(cpid, &status, 0);
}
