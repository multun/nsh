#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

#include "cli/cmdopts.h"


struct cmdopts g_cmdopts;


static struct option g_long_options[] =
{
  {"norc",        no_argument, &g_cmdopts.norc,   true},
  {"ast-print",   no_argument, &g_cmdopts.shmode, SHMODE_AST_PRINT},
  {"token-print", no_argument, &g_cmdopts.shmode, SHMODE_TOKEN_PRINT},
  {"version",     no_argument, &g_cmdopts.shmode, SHMODE_VERSION},
  {"help",        no_argument, 0,                 'h'},
  {0, 0, 0, 0}
};


static void print_help(char *pname)
{
  // TODO: exhaustive options
  char *base = strdup(pname);
  printf("Usage: %s [options] [script-file]\n",
         basename(base));
  free(base);
}


int cmdopts_parse(int argc, char *argv[])
{
  int opt_i = 0;
  int c;

  while ((c = getopt_long(argc, argv, "hvanc", g_long_options, &opt_i)) != -1)
    switch (c)
    {
    case 0:
      break;
    case 'c':
      g_cmdopts.src = SHSRC_COMMAND;
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
