#include <string.h>
#include <stdio.h>
#include <sys/random.h>
#include <unistd.h>

#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/evect.h"
#include "utils/alloc.h"


static void expand_pid(char **res)
{
  pid_t id =  getpid();
  *res = xcalloc(25, sizeof(char));
  sprintf(*res, "%u", id);
}

static void expand_args(char **res, s_env *env, bool split)
{
  s_evect vec;
  evect_init(&vec, 10);
  if (*env->argv)
    for (char *tmp = *env->argv; *tmp; tmp++)
      evect_push(&vec, *tmp);
  for (size_t i = 1; env->argv[i]; i++)
  {
    evect_push(&vec, ' ');
    for (char *tmp = *env->argv; *tmp; tmp++)
       evect_push(&vec, *tmp);
  }
  evect_push(&vec, '\0');
  *res = vec.data;
  if (split)
    return;
}


static void expand_sharp(char **res, s_env *env)
{
  size_t argc = 0;
  while (env->argv[argc])
    argc++;

  *res = xcalloc(25, sizeof(char));
  sprintf(*res, "%zu", argc);
}


static void expand_return(char **res, s_env *env)
{
  *res = xcalloc(25, sizeof(char));
  sprintf(*res, "%u", (256 + env->code) % 256);
}


bool special_char_lookup(char **res, s_env *env, char *var)
{
  switch (*var)
  {
  case '@':
    expand_args(res, env, true);
    return true;
  case '*':
    expand_args(res, env, false);
    return true;
  case '?':
    expand_return(res, env);
    return true;
  case '$':
    expand_pid(res);
    return true;
  case '#':
    expand_sharp(res, env);
    return true;
  default:
    return false;
  }
  return false;
}


void expand_random(char **res)
{
  int rnd = 0;
  getrandom(&rnd, sizeof(int), 0);
  *res = xcalloc(6, sizeof(char));
  sprintf(*res, "%d", (rnd % 32768 + 32768) % 32768);
}


void expand_uid(char **res)
{
  uid_t id =  getuid();
  *res = xcalloc(25, sizeof(char));
  sprintf(*res, "%u", id);
}
