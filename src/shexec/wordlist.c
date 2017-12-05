#include <stdlib.h>
#include <err.h>

#include "ast/wordlist.h"
#include "shexec/environment.h"
#include "utils/alloc.h"


static void expand(char *str, s_env *env)
{
  // TODO
  if (!str || !env)
    warnx("expand: not implemented yet.");
  return;
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
    argv[i] = wl->str;
    if (argv[i][0] == '$')
      expand(argv[i], env);
    wl = wl->next;
  }
  return argv;
}


void wordlist_free(s_wordlist *wl, bool free_buf)
{
  if (!wl)
    return;
  if (free_buf)
    free(wl->str);
  wordlist_free(wl->next, free_buf);
  free(wl);
}
