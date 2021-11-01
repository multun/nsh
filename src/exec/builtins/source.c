#include <nsh_exec/repl.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_utils/alloc.h>


#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

static int source_file(struct exception_catcher *catcher, struct environment *env,
                       char *path)
{
    int rc;
    FILE *file;

    if ((rc = cstream_file_setup(&file, path, false)))
        return rc;

    struct cstream_file cs;
    cstream_file_init(&cs, file, true);

    struct repl ctx;
    repl_init(&ctx, &cs.base, env);

    struct repl_result repl_res;
    repl_run(&repl_res, &ctx);
    rc = repl_status(&ctx);

    if (repl_called_exit(&repl_res))
        clean_exit(catcher, rc);

    repl_destroy(&ctx);
    cstream_destroy(&cs.base);
    return rc;
}

int builtin_source(struct environment *env, struct exception_catcher *catcher, int argc,
                   char **argv)
{
    if (argc > 2) {
        warnx("source: missing source");
        return 1;
    }

    return !!source_file(catcher, env, argv[1]);
}
