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
    ctx.cs = cstream_from_string(str, "<subshell>");
    repl(&ctx);
    int rc = ctx.env->code;
    ctx.env = NULL; // avoid double free
    context_destroy(&ctx);
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
void expand_subshell(struct errcont *cont, char *buf, s_env *env, struct evect *vec)
{
    int pfd[2];
    // TODO: handle errors
    pipe(pfd);
    int cpid = fork();
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
