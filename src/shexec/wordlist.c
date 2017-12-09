#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "ast/wordlist.h"
#include "shexec/environment.h"
#include "utils/alloc.h"
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


static void expand_var(char **str, s_env *env, s_evect *vec)
{
  bool braces = **str == '{';
  if (braces)
    (*str)++;

  s_evect var;
  evect_init(&var, strlen(*str));
  for (; **str && (!braces || **str != '}'); (*str)++)
    evect_push(&var, **str);

  char *res = htable_access(env->vars, var.data);
  evect_destroy(&var);

  if (res)
    for (; *res; res++)
      evect_push(vec, *res);
  if (braces)
    (*str)++;
}


static char *expand(char *str, s_env *env)
{
  s_evect vec;
  evect_init(&vec, strlen(str));
  for (; *str; str++)
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
      evect_push(&vec, *str);
  }
  evect_push(&vec, '\0');
  return vec.data;
}


char **wordlist_to_argv(s_wordlist *wl, s_env *env)
{
  size_t argc = 0;
  s_wordlist *tmp = wl;
  while (tmp)
  {
    argc++;
    tmp = tmp->next;
  }

  char **argv = xmalloc(sizeof (char*) * (argc + 1));
  argv[argc] = NULL;
  for (size_t i = 0; i < argc; i++)
  {
    argv[i] = expand(wl->str, env);
    wl = wl->next;
  }
  return argv;
}


void wordlist_free(s_wordlist *wl, bool free_buf)
{
  if (!wl)
    return;
  if (free_buf && wl->str)
    free(wl->str);
  wordlist_free(wl->next, free_buf);
  free(wl);
}
