#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>

#include "shexec/builtins.h"
#include "utils/alloc.h"


static void update_pwd(void)
{
  setenv("OLDPWD", getenv("PWD"), 1);
  char *buf = xcalloc(PATH_MAX, sizeof(char));
  size_t size = 50;
  if (!getcwd(buf, size))
  {
    free(buf);
    buf = NULL;
    return;
  }
  setenv("PWD", buf, 1);
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
  else if (chdir(path) != 0)
  {
    warn("cd: chdir failed");
    return 1;
  }
  update_pwd();
  return 0;
}

int builtin_cd(int argc, char **argv)
{
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
    if (chdir(argv[1]) != 0)
    {
      warn("cd: chdir failed");
      return 1;
    }
    update_pwd();
  }
  return 0;
}
