#include <stdlib.h>

#include "ast/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/unquote.h"
#include "utils/alloc.h"


static size_t wordlist_argc_count(s_wordlist *wl)
{
  size_t argc = 0;
  for (s_wordlist *tmp = wl; tmp; tmp = tmp->next)
    argc++;
  return argc;
}


static void wordlist_to_argv_sub(char **volatile *res, s_wordlist *volatile wl,
                                 s_env *env, s_errcont *cont)
{
  size_t argc = wordlist_argc_count(wl);
  char **argv = *res = calloc(sizeof (char*), (argc + 1));
  for (volatile size_t i = 0; i < argc; (wl = wl->next), i++)
  {
    char *volatile expanded = NULL;
    s_keeper keeper = KEEPER(cont->keeper);
    if (setjmp(keeper.env))
    {
      free(expanded);
      shraise(cont, NULL);
    }
    else
    {
      s_errcont ncont = ERRCONT(cont->errman, &keeper);
      // TODO: handle expansion interuptions
      expanded = expand(wl->str, env, &ncont);
      unquote(&argv[i], expanded, &ncont);
      free(expanded);
    }
  }
}


// this function is here just in case res == &env->argv, so that expansion of
// the parameters is done within the proper context
void wordlist_to_argv(char ***res, s_wordlist *wl, s_env *env, s_errcont *cont)
{
  char **volatile new_argv = NULL;
  s_keeper keeper = KEEPER(cont->keeper);
  if (setjmp(keeper.env))
  {
    argv_free(new_argv);
    shraise(cont, NULL);
  }
  else
  {
    wordlist_to_argv_sub(&new_argv, wl, env, &ERRCONT(cont->errman, &keeper));
    *res = new_argv;
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
