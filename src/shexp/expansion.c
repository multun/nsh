#include <assert.h>
#include <string.h>
#include <err.h>

#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/evect.h"


static void expand_subshell(char **str, s_env *env, s_evect *vec)
{
  if (!*str && !env && !vec)
    warnx("expand_subshell: not implemented yet");
}


static void expand_arth(char **str, s_env *env, s_evect *vec)
{
  if (!*str && !env && !vec)
    warnx("expand_arth: not implemented yet");
}


static char *var_lookup(s_env *env, char *var)
{
  struct pair *var_pair = htable_access(env->vars, var);
  if (!var_pair)
    return NULL;

  return var_pair->value;
}


static void expand_var(char **str, s_env *env, s_evect *vec)
{
  bool braces = **str == '{';
  if (braces)
    (*str)++;

  s_evect var;
  evect_init(&var, strlen(*str));
  for (; **str && (!braces || **str != '}'); (*str)++)
    evect_push(&var, **str);
  evect_push(&var, '\0');

  char *res = var_lookup(env, var.data);
  evect_destroy(&var);

  if (res)
    for (; *res; res++)
      evect_push(vec, *res);
  if (braces)
  {
    assert(**str == '}');
    (*str)++;
  }
}


char *expand(char *str, s_env *env)
{
  s_evect vec;
  evect_init(&vec, strlen(str) + 1);
  while (*str)
  {
    if (*str == '$' && str[1] && str++)
    {
      if (*str == '(' && str++)
      {
        if (*str == '(')
          expand_arth(&str, env, &vec);
        else
          expand_subshell(&str, env, &vec);
      }
      else
        expand_var(&str, env, &vec);
    }
    else
      evect_push(&vec, *(str++));
  }
  evect_push(&vec, '\0');
  return vec.data;
}
