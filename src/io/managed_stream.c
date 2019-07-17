#include "io/managed_stream.h"
#include "io/cstream.h"
#include "repl/repl.h"
#include "cli/cmdopts.h"
#include "repl/repl.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool is_interactive(int argc)
{
    return (g_cmdopts.src != SHSRC_COMMAND) && argc <= 0 && isatty(STDIN_FILENO);
}

static int init_command(s_cstream **cs, int argc, char *path)
{
    if (argc == 0) {
        warnx("missing command");
        return 2;
    }

    *cs = cstream_from_string(path, "<command line>", NULL);
    (*cs)->interactive = false;
    (*cs)->context = NULL;
    return 0;
}

static int init_file(s_cstream **cs, char *path)
{
    FILE *file;
    if (!(file = fopen(path, "r"))) {
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

    *cs = cstream_from_file(file, path, true);
    (*cs)->interactive = false;
    (*cs)->context = NULL;
    return 0;
}

int cstream_dispatch_init(s_context *context, s_cstream **cs, s_arg_context *arg_cont)
{
    char *first_arg = arg_cont->argv[arg_cont->argc_base];
    int rebased_argc = arg_cont->argc - arg_cont->argc_base;
    if (g_cmdopts.src == SHSRC_COMMAND) {
        // we need to increase the base so that the other
        // routines properly consider arguments
        int new_progind = arg_cont->argc_base + 1;
        if (arg_cont->argv[new_progind])
            arg_cont->progname_ind = new_progind;
        arg_cont->argc_base += 1 + !!arg_cont->argv[new_progind];
        return init_command(cs, rebased_argc, first_arg);
    }

    if (rebased_argc >= 1)
        return init_file(cs, first_arg);

    if (is_interactive(rebased_argc)) {
        *cs = cstream_readline();
        (*cs)->interactive = true;
        (*cs)->context = context;
    } else {
        *cs = cstream_from_file(stdin, "<stdin>", false);
        (*cs)->interactive = false;
        (*cs)->context = NULL;
    }
    return 0;
}
