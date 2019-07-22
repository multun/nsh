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
    struct context ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = env;

    FILE *file;
    int res;
    if ((res = cstream_file_setup(&file, path, false)))
        return res;

    struct cstream_file cs;
    cstream_file_init(&cs, file, true);
    ctx.cs = &cs.base;
    if (repl(&ctx))
        clean_exit(cont, ctx.env->code);

    int rc = ctx.env->code;
    ctx.env = NULL; // avoid double free
    context_destroy(&ctx);
    cstream_destroy(&cs.base);
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
