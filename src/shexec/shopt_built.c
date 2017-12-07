#include <cli/shopt.h>


size_t parse_builtin_shopt_opt(bool *opt, char **argv)
{
  size_t index = 1;
  while (argv[index] && *(argv[index]) == '-')
  {
    if (!strcmp(argv[index], "-q"))
      opt[0] = false;
    else if (!strcmp(argv[index], "-s"))
    {
      if (opt[2])
      {
        warnx("shopt: can not set and unset");
        return 0;
      }
      opt[1] = true;
    }
    else if (!strcmp(argv[index], "-u"))
    {
      if (opt[1])
      {
        warnx("shopt: can not set and unset");
        return 0;
      }
      opt[2] = true;
    }
    index++;
  }
  return index;
}


int builtin_shopt(int argc, char **argv)
{
  bool *opt =
  {
    true; //print
    false; //reusable
    false; //set
    false; //unset
  };
  if (!parse_builtin_shopt_opt(opt, argv)
    return 2;
}
