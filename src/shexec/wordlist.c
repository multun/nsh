#include <stdlib.h>

#include "ast/wordlist.h"
#include "utils/alloc.h"


char **wordlist_to_argc(s_wordlist *wl)
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
    wl = wl->next;
  }
  return argv;
}
