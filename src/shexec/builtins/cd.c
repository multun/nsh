#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>

#include "shexec/builtins.h"
#include "utils/alloc.h"
#include "ast/assignment.h"
#include "utils/hash_table.h"
#include "shexp/variable.h"


void update_pwd(bool oldpwd, s_env *env)
{
  char *buf = xcalloc(PATH_MAX, sizeof(char));
  size_t size = PATH_MAX;
  if (!getcwd(buf, size))
  {
    free(buf);
    buf = NULL;
    return;
  }
  char *pwd = strdup(oldpwd ? "OLDPWD" : "PWD");
  assign_var(env, pwd, buf, true);
}


static int cd_from_env(const char *env_var, s_env *env)
{
  struct pair *p = htable_access(env->vars, env_var);
  char *path = NULL;
  if (p && p->value)
  {
    s_var *node = p->value;
    path = node->value;
    if (!strcmp("OLDPWD", env_var))
      path = strdup(path);
  }
  if (!path)
  {
    warnx("cd: no %s set", env_var);
    return 1;
  }
  update_pwd(true, env);
  if (chdir(path) != 0)
  {
    warn("cd: chdir failed");
    return 1;
  }
  update_pwd(false, env);
  return 0;
}

static int cd_with_minus(s_env *env)
{
  int res = cd_from_env("OLDPWD", env);

  if (!res)
  {
    char *buf = xcalloc(PATH_MAX, sizeof(char));
    size_t size = PATH_MAX;
    if (!getcwd(buf, size))
    {
      free(buf);
      return 1;
    }
    printf("%s\n", buf);
    free(buf);
  }
  return res;
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
    return cd_from_env("HOME", env);
  else if (!strcmp(argv[1], "-"))
    return cd_with_minus(env);
  else
  {
    update_pwd(true, env);
    if (chdir(argv[1]) != 0)
    {
      warn("cd: chdir failed");
      return 1;
    }
    update_pwd(false, env);
  }
  return 0;
}
