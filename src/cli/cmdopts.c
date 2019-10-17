#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "cli/cmdopts.h"
#include "cli/shopt.h"
#include "gen/config.h"

struct cmdopts g_cmdopts;

static struct option g_long_options[] = {
    {"norc", no_argument, &g_cmdopts.norc, true},
    {"ast-print", no_argument, &g_shopts[SHOPT_AST_PRINT], true},
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

static void print_help(char *pname)
{
    // TODO: exhaustive options
    char *base = strdup(pname);
    printf("Usage: %s [options] [script-file]\n", basename(base));
    free(base);
}

static void preprocess_cmdline(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
        if (!strcmp(argv[i], "+O"))
            strcpy(argv[i], "-o");
        else if (!strcmp(argv[i], "--") || argv[i][0] != '-')
            break;
}

static bool handle_shopt(bool val, const char *str)
{
    enum shopt cur_shopt = shopt_from_string(str);
    if (cur_shopt == SHOPT_COUNT) {
        warnx("cannot find shopt %s", str);
        return true;
    }
    g_shopts[cur_shopt] = val;
    return false;
}

static int handle_version(void)
{
    puts("Version " VERSION);
    return -1;
}

int cmdopts_parse(int argc, char *argv[])
{
    int opt_i = 0;
    int c;

    preprocess_cmdline(argc, argv);
    while ((c = getopt_long(argc, argv, "+hvanc:o:O:", g_long_options, &opt_i)) != -1)
        switch (c) {
        case 0:
            break;
        case 'v':
            return handle_version();
        case 'c':
            g_cmdopts.src = SHSRC_COMMAND;
            g_cmdopts.command = optarg;
            break;
        case 'o':
        case 'O':
            if (handle_shopt(c == 'O', optarg))
                return -3;
            break;
        case 'h':
            print_help(argv[0]);
            return -1;
        default:
            /* on error or unexpected code,
            ** let the caller cleanup
            */
            return -2;
        }

    // optind is the internal getopt cursor
    return optind;
}
