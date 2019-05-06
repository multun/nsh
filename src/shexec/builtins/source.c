#include "repl/repl.h"
#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "utils/alloc.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

static int source_file(s_errcont *cont, s_env *env, char *path)
{
    s_context ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = env;
    FILE *file;
    if (!(file = fopen(path, "r+"))) {
        int res = errno;
        // TODO: check the return code is right
        warn("cannot open input script");
        return res;
    }

    if (fcntl(fileno(file), F_SETFD, FD_CLOEXEC) < 0) {
        int res = errno;
        warn("couldn't set CLOEXEC on input file %d", fileno(file));
        return res;
    }

    ctx.cs = cstream_from_file(file, path, true);
    if (repl(&ctx))
        clean_exit(cont, ctx.env->code);

    int rc = ctx.env->code;
    ctx.env = NULL; // avoid double free
    context_destroy(&ctx);
    return rc;
}

int builtin_source(s_env *env, s_errcont *cont, int argc, char **argv)
{
    if (argc > 2) {
        warnx("source: missing source");
        return 1;
    }

    return !!source_file(cont, env, argv[1]);
}
