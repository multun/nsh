#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>

#include "shexec/builtins.h"
#include "utils/alloc.h"


static void update_pwd(const char *env_var)
{
  char *buf = xcalloc(PATH_MAX, sizeof(char));
  size_t size = PATH_MAX;
  if (!getcwd(buf, size))
  {
    free(buf);
    buf = NULL;
    return;
  }
  setenv(env_var, buf, 1);
  free(buf);
}


static int cd_from_env(const char *env_var)
{
  char *path = getenv(env_var);
  if (!path)
  {
    warnx("cd: no %s set", env_var);
    return 1;
  }
  update_pwd("OLDPWD");
  if (chdir(path) != 0)
  {
    warn("cd: chdir failed");
    return 1;
  }
  update_pwd("PWD");
  return 0;
}


int builtin_cd(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (!env || !cont)
    warnx("cd: missing context elements");

  if (argc > 2)
  {
    warnx("cd: too many arguments");
    return 1;
  }

  if (argc == 1)
    return cd_from_env("HOME");
  else if (!strcmp(argv[1], "-"))
    return cd_from_env("OLDPWD");
  else
  {
    update_pwd("OLDPWD");
    if (chdir(argv[1]) != 0)
    {
      warn("cd: chdir failed");
      return 1;
    }
    update_pwd("PWD");
  }
  return 0;
}
