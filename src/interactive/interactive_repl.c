#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>
#include <nsh_utils/alloc.h>
#include <nsh_interactive/interactive_repl.h>
#include <readline/history.h>
#include <nsh_exec/history.h>
#include <nsh_utils/pathutils.h>

#include "cstream_dispatch.h"
#include "history.h"


static f_builtin find_builtin_with_history(const char *name)
{
    if (strcmp(name, "history") == 0)
        return builtin_history;
    return find_default_builtin(name);
}


static nsh_err_t load_rc(struct environment *env, const char *path, const char *source)
{
    nsh_err_t err;
    struct repl ctx;

    FILE *file = fopen(path, "r");
    if (cstream_file_setup(&file, path, true))
        return NSH_OK; // not being able to load an rc file is ok

    struct cstream_file cs;
    cstream_file_init(&cs, file);
    cs.base.line_info = LINEINFO(source, NULL);

    repl_init(&ctx, &cs.base, env);

    err = repl_run(&ctx);

    repl_destroy(&ctx);
    cstream_destroy(ctx.cs);

    /* exiting in an rc file causes the shell to exit */
    if (err == NSH_EXIT_INTERUPT)
        return err;
    return NSH_OK;
}

static nsh_err_t load_all_rc(struct repl *ctx)
{
    nsh_err_t err;
    const char global_rc[] = "/etc/nshrc";
    if ((err = load_rc(ctx->env, global_rc, global_rc)))
        return err;

    char *rc_path = home_filepath(".nshrc");
    if (rc_path == NULL)
        return NSH_OK;
    err = load_rc(ctx->env, rc_path, "~/.nshrc");
    free(rc_path);
    return err;
}

// these escaped are used by terminal emulators to get the title of the window
#define WINDOW_TITLE_START "\\[\\e]0;"
#define WINDOW_TITLE_END "\\a\\]"
#define ANSI_GRAY "\\[\\033[1;90m\\]"
#define ANSI_RESET "\\[\\033[0m\\]"
static const char default_ps1[] = {WINDOW_TITLE_START
                                   "\\u@\\h: \\w" WINDOW_TITLE_END ANSI_GRAY
                                   "[\\u@\\h:\\w]\\$ " ANSI_RESET};


static nsh_err_t repl_load(int *statuscode, struct repl *ctx, struct cstream *cs,
                           struct cli_options *arg_ctx)
{
    nsh_err_t err;
    struct environment *env = environment_load(arg_ctx);
    repl_init(ctx, cs, env);
    environment_put(env);

    // skip loading RC files and setting up the history
    // if the shell isn't interactive
    if (!ctx->cs->interactive)
        return NSH_OK;

    // settup the default PS1 (if running in the interactive mode)
    environment_var_assign_const_cstring(env, strdup("PS1"), default_ps1, false);

    // if loading the RC files failed, return the status code of the repl
    if (!arg_ctx->norc && (err = load_all_rc(ctx))) {
        *statuscode = ctx->env->code;
        return err;
    }

    // this is part of readline
    using_history();
    history_init(ctx, history_open());
    return NSH_OK;
}


int interactive_repl_init(struct repl *repl, struct cli_options *options,
                          struct cstream **cs)
{
    int rc;

    /* initialize IO */
    if ((rc = cstream_dispatch_init(repl, cs, options)))
        goto err_cstream;

    /* initialize the context the repl will work with */
    if (repl_load(&rc, repl, *cs, options))
        goto err_context;

    repl->env->find_builtin = find_builtin_with_history;
    repl->add_history = add_history;
    return 0;

err_context:
    cstream_free(*cs);
err_cstream:
    return rc;
}
