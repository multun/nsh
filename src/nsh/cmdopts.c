#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "shexec/repl.h"
#include "shexec/shopt.h"
#include "config.h"


static void print_help(char *pname)
{
    // TODO: exhaustive options
    char *base = strdup(pname);
    printf("Usage: %s [options] [script-file]\n", basename(base));
    free(base);
}

static bool handle_shopt(struct cli_options *res, bool val, const char *str)
{
    enum shopt cur_shopt = shopt_from_string(str);
    if (cur_shopt == SHOPT_COUNT) {
        warnx("cannot find shopt %s", str);
        return true;
    }
    res->shopts[cur_shopt] = val;
    return false;
}

int parse_cli_options(struct cli_options *res, int argc, char *argv[])
{
    memset(res, 0, sizeof(*res));
    res->argc = argc;
    res->argv = argv;
    res->progname_ind = 0;
    res->src = SHSRC_FILE;
    res->norc = false;

    int print_ast = false;
    struct option long_options[] = {
        {"norc", no_argument, &res->norc, true},
        {"ast-print", no_argument, &print_ast, true},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // replace +O into -o so getopt_long can process it
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "+O") == 0)
            strcpy(argv[i], "-o");
        else if (!strcmp(argv[i], "--") || argv[i][0] != '-')
            break;

    int c;
    while ((c = getopt_long(argc, argv, "+hvanc:o:O:", long_options, NULL)) != -1)
        switch (c) {
        case 0:
            break;
        case 'v':
            puts("Version " VERSION);
            return 0;
        case 'c':
            res->src = SHSRC_COMMAND;
            res->command = optarg;
            break;
        case 'o':
        case 'O':
            if (handle_shopt(res, c == 'O', optarg))
                return 2;
            break;
        case 'h':
            print_help(argv[0]);
            return 0;
        default:
            /* on error or unexpected code,
            ** let the caller cleanup
            */
            return 1;
        }

    // optind is the internal getopt cursor
    res->argc_base = optind;

    if (print_ast)
        res->shopts[SHOPT_AST_PRINT] = true;
    return 0;
}
