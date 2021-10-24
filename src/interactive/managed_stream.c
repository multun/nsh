#include <nsh_interactive/managed_stream.h>
#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>
#include <nsh_utils/alloc.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int cstream_dispatch_init_unwrapped(struct cstream **cs, struct cli_options *arg_cont, int remaining_argc)
{
    if (arg_cont->src == SHSRC_COMMAND) {
        struct cstream_string *res = zalloc(sizeof(*res));
        cstream_string_init(res, arg_cont->command);
        res->base.line_info = LINEINFO("<command line>", NULL);
        *cs = &res->base;
        return 0;
    }

    if (remaining_argc) {
        char *program_name = arg_cont->argv[arg_cont->progname_ind];
        FILE *file;
        int errcode;
        if ((errcode = cstream_file_setup(&file, program_name, false)))
            return errcode;

        struct cstream_file *res = zalloc(sizeof(*res));
        cstream_file_init(res, file, true);
        res->base.line_info = LINEINFO(program_name, NULL);
        *cs = &res->base;
        return 0;
    }

    if (isatty(STDIN_FILENO)) {
        struct cstream_readline *res = zalloc(sizeof(*res));
        cstream_readline_init(res);
        res->base.line_info = LINEINFO("<interactive input>", NULL);
        *cs = &res->base;
        return 0;
    }

    struct cstream_file *res = zalloc(sizeof(*res));
    cstream_file_init(res, stdin, false);
    res->base.line_info = LINEINFO("<stdin>", NULL);
    *cs = &res->base;
    return 0;
}

int cstream_dispatch_init(struct repl *context, struct cstream **cs, struct cli_options *arg_cont)
{
    int remaining_argc = arg_cont->argc - arg_cont->argc_base;

    // if there are remaining arguments after options parsing,
    // set the program name to the first argument and higher the base
    if (remaining_argc) {
        arg_cont->progname_ind = arg_cont->argc_base;
        arg_cont->argc_base++;
    }

    int errcode = cstream_dispatch_init_unwrapped(cs, arg_cont, remaining_argc);
    if (errcode != 0)
        return errcode;

    (*cs)->context = context;
    cstream_check(*cs);
    return 0;
}
