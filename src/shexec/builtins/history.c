#include "repl/history.h"
#include "shexec/builtins.h"
#include "utils/alloc.h"

#include <stdio.h>
#include <readline/history.h>
#include <err.h>
#include <string.h>

static int history_format(s_env *env)
{
  HISTORY_STATE *hstate = history_get_history_state();
  for (int i = 0; i < hstate->length; i++)
    printf("%4d  %s\n", i + 1, hstate->entries[i]->line);
  return !env;
}


static int history_clear(void)
{
  clear_history();
  return 0;
}


static int history_reload(void)
{
  clear_history();
  FILE *hist = history_open();
  if (!hist)
    return 1;

  size_t size = 0;
  char *lineptr = NULL;

  while (getline(&lineptr, &size, hist) != -1)
  {
    char *cur_line = strndup(lineptr, size ? size - 1 : 0);
    add_history(cur_line);
    free(cur_line);
  }
  free(lineptr);
  fclose(hist);
  return 0;
}


int builtin_history(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (argc == 1 && cont)
    return history_format(env);

  for (int i = 1; i < argc; i++)
    if (argv[i][0] != '-')
      continue;
    else
      for (int ci = 1; argv[i][ci]; ci++)
        switch (argv[i][ci])
        {
        case 'r':
          history_reload();
          break;
        case 'c':
          history_clear();
          break;
        default:
          warnx("history: unknown option %c", argv[i][ci]);
          return 1;
        }

  return 0;
}
