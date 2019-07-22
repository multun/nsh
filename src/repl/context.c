#include "ast/ast_list.h"
#include "cli/cmdopts.h"
#include "io/cstream.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/environment.h"
#include "utils/pathutils.h"

#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool context_load_rc(s_env *env, const char *path, const char *source)
{
    s_context cont;
    memset(&cont, 0, sizeof(cont));
    FILE *file = fopen(path, "r");
    int res;
    if ((res = cstream_file_setup(&file, path, true)))
        return 0; // not being able to load an rc file is ok

    struct cstream_file cs = { 0 };
    cstream_file_init(&cs, file, true);
    cont.cs = &cs.base;
    cont.env = env;
    cs.base.line_info = LINEINFO(source, NULL);
    cs.base.context = &cont;

    bool should_exit = repl(&cont);

    cstream_destroy(cont.cs);
    return should_exit;
}

static bool context_load_all_rc(s_context *cont)
{
    const char global_rc[] = "/etc/42shrc";
    if (context_load_rc(cont->env, global_rc, global_rc))
        return true;

    char *rc_path = home_suffix("/.42shrc");
    bool should_exit = context_load_rc(cont->env, rc_path, "~/.42shrc");
    free(rc_path);
    return should_exit;
}

bool context_init(int *rc, s_context *cont, s_arg_context *arg_cont)
{
    memset(cont, 0, sizeof(*cont));

    if ((*rc = cstream_dispatch_init(cont, &cont->cs, arg_cont)))
        return true;

    cont->env = environment_create(arg_cont);

    if (cont->cs->interactive && !g_cmdopts.norc && context_load_all_rc(cont)) {
        *rc = cont->env->code;
        return true;
    }

    history_init(cont);
    return 0;
}

void context_destroy(s_context *cont)
{
    environment_free(cont->env);
    history_destroy(cont);
}
