#include <stdlib.h>

#include "ast/wordlist.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/unquote.h"
#include "utils/alloc.h"


void wordlist_to_argv(char ***res, s_wordlist *wl, s_env *env, s_errcont *cont)
{
  size_t argc = 0;
  s_wordlist *tmp = wl;
  while (tmp)
  {
    argc++;
    tmp = tmp->next;
  }

  char **argv = *res = calloc(sizeof (char*), (argc + 1));

  for (size_t i = 0; i < argc; (wl = wl->next), i++)
  {
    char *expanded = expand(wl->str, env, cont);
    argv[i] = unquote(expanded);
    free(expanded);
  }
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
