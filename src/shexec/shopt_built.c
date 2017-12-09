#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cli/shopt.h"


bool parse_builtin_shopt_opt(bool *opt, char **argv, int *index)
{
  for (; argv[*index] && *(argv[*index]) == '-'; (*index)++)
    if (!strcmp(argv[*index], "-q"))
      opt[0] = false;
    else if (!strcmp(argv[*index], "-s"))
    {
      if (opt[2])
      {
        warnx("shopt: can not set and unset");
        return false;
      }
      opt[1] = true;
    }
    else if (!strcmp(argv[*index], "-u"))
    {
      if (opt[1])
      {
        warnx("shopt: can not set and unset");
        return false;
      }
      opt[2] = true;
    }
    else
      return false;
  return true;
}


static void print_shopt(bool *opt, int argc, char **argv, int index)
{
  if (!opt[0])
    return;
  if (index != argc)
    for (; index < argc; index++)
      printf("%s\t%s", argv[index],
             g_shopts[shopt_from_string(argv[index])] ? "on": "off");
  else
    for (size_t i = 0; i < SHOPT_COUNT; i++)
    {
      if ((g_shopts[i] && (opt[1] || !opt[2]))
          || (!g_shopts[i] && (!opt[1] || opt[2])))
        printf("%s\t%s", string_from_shopt(i), g_shopts[i] ? "on": "off");
    }
}


int builtin_shopt(int argc, char **argv)
{
  bool opt[3] =
  {
    0,
  };
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
