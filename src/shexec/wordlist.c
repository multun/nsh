#include <stdlib.h>

#include "ast/wordlist.h"
#include "shexec/environment.h"
#include "utils/alloc.h"


static void expand(char *str, s_env *env)
{
  str = str;
  env = env;
  // TODO
}

char **wordlist_to_argc(s_wordlist *wl, s_env *env)
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
    if (argv[i][0] == '$')
      expand(argv[i], env);

    argv[i] = wl->str;
    wl = wl->next;
  }
  return argv;
}
