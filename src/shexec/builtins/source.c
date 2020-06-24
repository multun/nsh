#include "repl/repl.h"
#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "utils/alloc.h"


#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

static int source_file(struct errcont *cont, struct environment *env, char *path)
{
    int rc;
    FILE *file;

    if ((rc = cstream_file_setup(&file, path, false)))
        return rc;

    struct cstream_file cs;
    cstream_file_init(&cs, file, true);

    struct context ctx;
    context_from_env(&ctx, &cs.base, env);

    struct repl_result repl_res;
    repl(&repl_res, &ctx);
    rc = repl_status(&ctx);

    if (repl_called_exit(&repl_res))
        clean_exit(cont, rc);

    context_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

int builtin_source(struct environment *env, struct errcont *cont, int argc, char **argv)
{
    if (argc > 2) {
        warnx("source: missing source");
        return 1;
    }

    return !!source_file(cont, env, argv[1]);
}
