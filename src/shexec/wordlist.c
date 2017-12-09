#include <stdlib.h>

#include "ast/wordlist.h"
#include "shexec/environment.h"
#include "shexec/expansion.h"
#include "utils/alloc.h"


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

  for (size_t i = 0; i < argc; (wl = wl->next), i++)
  {
    char *expanded = expand(wl->str, env);
    argv[i] = unquote(expanded);
    free(expanded);
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
