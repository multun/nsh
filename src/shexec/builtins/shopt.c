#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cli/shopt.h"
#include "shexec/builtins.h"
#include "utils/evect.h"

enum shopt_options
{
  SHOPT_OPT_QUIET,
  SHOPT_OPT_SET,
  SHOPT_OPT_UNSET,
  SHOPT_OPT_COUNT,
};

typedef bool t_shopt_options[SHOPT_OPT_COUNT];

#define SHOPT_OPT_DEFAULT                       \
  {                                             \
    0,                                          \
  }


bool parse_builtin_shopt_opt(t_shopt_options opts, char **argv, int *index)
{
  for (; argv[*index] && *(argv[*index]) == '-'; (*index)++)
    if (!strcmp(argv[*index], "-q"))
      opts[SHOPT_OPT_QUIET] = false;
    else if (!strcmp(argv[*index], "-s"))
    {
      if (opts[SHOPT_OPT_UNSET])
      {
        warnx("shopt: can not set and unset");
        return false;
      }
      opts[SHOPT_OPT_SET] = true;
    }
    else if (!strcmp(argv[*index], "-u"))
    {
      if (opts[SHOPT_OPT_SET])
      {
        warnx("shopt: can not set and unset");
        return false;
      }
      opts[SHOPT_OPT_UNSET] = true;
    }
    else
      return false;
  return true;
}


static void print_shopt(t_shopt_options opts, int argc, char **argv, int index)
{
  if (opts[SHOPT_OPT_QUIET])
    return;

  if (index != argc)
    for (; index < argc; index++)
      printf("%s\t%s\n", argv[index],
             g_shopts[shopt_from_string(argv[index])] ? "on": "off");
  else
    for (size_t i = 0; i < SHOPT_COUNT; i++)
      if ((g_shopts[i] && (opts[SHOPT_OPT_SET] || !opts[SHOPT_OPT_UNSET]))
          || (!g_shopts[i] && (!opts[SHOPT_OPT_SET] || opts[SHOPT_OPT_UNSET])))
        printf("%s\t%s\n", string_from_shopt(i), g_shopts[i] ? "on": "off");
}


int builtin_shopt(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (!env || !cont)
    warnx("cd: missing context elements");

  t_shopt_options opt = SHOPT_OPT_DEFAULT;

  int index = 1;
  if (!parse_builtin_shopt_opt(opt, argv, &index))
    return 2;

  for (int i = index; i < argc; i++)
    if (shopt_from_string(argv[i]) == SHOPT_COUNT)
    {
      warnx("shopt: %s: invalid shell option name", argv[i]);
      return 1;
    }

  if (argc == index || (!opt[1] && !opt[2]))
  {
    print_shopt(opt, argc, argv, index);
    return 0;
  }

  for (; index < argc; index++)
    g_shopts[shopt_from_string(argv[index])] = opt[1];
  return 0;
}


void expand_shopt(char **res)
{
  s_evect vec;
  evect_init(&vec, 50);
  bool first = true;
  for (size_t i = 0; i < SHOPT_COUNT; i++)
    if (g_shopts[i])
    {
      if (!first)
        evect_push(&vec, ':');
      for (const char *tmp = string_from_shopt(i); *tmp; tmp++)
        evect_push(&vec, *tmp);
      first = false;
    }
  *res = vec.data;
}
