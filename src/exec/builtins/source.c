#include <nsh_exec/repl.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_utils/alloc.h>


#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

static nsh_err_t source_file(struct environment *env, char *path)
{
    int rc;
    FILE *file;

    if ((rc = cstream_file_setup(&file, path, false)))
        goto error;

    struct cstream_file cs;
    cstream_file_init(&cs, file);

    struct repl ctx;
    repl_init(&ctx, &cs.base, env);

    struct repl_result repl_res;
    repl_run(&repl_res, &ctx);
    rc = repl_status(&ctx);

    if (repl_called_exit(&repl_res))
        return clean_exit(env, rc);

    repl_destroy(&ctx);
    cstream_destroy(&cs.base);

error:
    env->code = rc;
    return NSH_OK;
}

nsh_err_t builtin_source(struct environment *env, int argc, char **argv)
{
    if (argc > 2) {
        warnx("source: missing source");
        env->code = 1;
        return NSH_OK;
    }

    return source_file(env, argv[1]);
}
