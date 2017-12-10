#include <err.h>

#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "utils/alloc.h"



int builtin_exit(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (!env)
    warnx("cd: missing context elements");

  if (argc > 2)
  {
    warnx("cd: too many arguments");
    return 1;
  }

  clean_exit(cont, argc == 2 ? atoi(argv[1]) : env->code);
}
