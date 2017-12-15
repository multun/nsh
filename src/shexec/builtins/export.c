#include <err.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>

#include "ast/assignment.h"
#include "shexec/builtins.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/alloc.h"


static void unexport_var(s_env *env, char *name)
{
  struct pair *prev = htable_access(env->vars, name);
  if (prev)
  {
    s_var *var = prev->value;
    var->to_export = false;
  }
}


static int export_var(s_env *env, char *entry, bool remove, s_errcont *cont)
{
  char *var = expand(entry, env, cont);
  char *word = NULL;
  char *name = strdup(strtok_r(var, "=", &word));
  regex_t regex;
  if (regcomp(&regex, "^[[:alpha:]_][[:alnum:]_]*$", REG_EXTENDED))
  {
    warnx("export: an error occurs compiling the regex");
    return 1;
  }
  if (entry[0] == '=' || regexec(&regex, name, 0, NULL, 0) == REG_NOMATCH)
  {
    warnx("export: '%s': not a valid identifier", entry);
    regfree(&regex);
    free(var);
    return 1;
  }
  if (remove)
    unexport_var(env, name);
  else if (*word == '\0' && *(word - 1) != '=')
    assign_var(env, name, NULL, true);
  else
    assign_var(env, name, expand(word, env, cont), true);
  regfree(&regex);
  free(var);
  return 0;
}


static void export_print(s_env *env)
{
  s_htable *vars = env->vars;
  for (size_t i = 0; i < vars->capacity; i++)
  {
    struct pair *pair = vars->tab[i];
    while (pair)
    {
      s_var *var = pair->value;
      if (var->to_export && var->value)
        printf("export %s=\"%s\"\n", pair->key, var->value);
      else if (var->to_export) printf("export %s\n", pair->key);
      pair = pair->next;
    }
  }
}




int builtin_export(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (!env || !cont)
    warnx("export: missing context elements");
  int res = 0;
  bool print = true;
  bool remove = false;
  for (int i = 1; i < argc; i++)
  {
    if (!strcmp("-n", argv[i]))
      remove = true;
    else if (strcmp("-p", argv[i]))
    {
      print = false;
      res |= export_var(env, argv[i], remove, cont);
    }
  }
  if (print)
    export_print(env);
  return res;
}
