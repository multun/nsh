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

static int init_command(s_cstream **cs)
{
    *cs = cstream_from_string(g_cmdopts.command, "<command line>", NULL);
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
    int remaining_argc = arg_cont->argc - arg_cont->argc_base;

    // if there are remaining arguments after options parsing,
    // set the program name to the first argument and higher the base
    if (remaining_argc) {
        arg_cont->progname_ind = arg_cont->argc_base;
        arg_cont->argc_base++;
    }

    if (g_cmdopts.src == SHSRC_COMMAND)
        return init_command(cs);

    if (remaining_argc) {
        char *program_name = arg_cont->argv[arg_cont->progname_ind];
        return init_file(cs, program_name);
    }

    if (isatty(STDIN_FILENO)) {
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
