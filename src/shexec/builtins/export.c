#include <err.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>

#include "shexec/builtins.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/alloc.h"


static void export_value(s_env *env, char *name, char* value)
{
    struct pair *prev = htable_access(env->vars, name);
    if (prev)
    {
      s_var *v = prev->value;
      v->to_export = true;
      v->value = value;
      return;
    }
    s_var *new = xmalloc(sizeof(*new));
    new->to_export = true;
    new->touched = true;
    new->value = value;
    htable_add(env->vars, name, new);
}


static void export_novalue(s_env *env, char *name)
{
  struct pair *prev = htable_access(env->vars, name);
  if (prev)
  {
    s_var *v = prev->value;
    v->to_export = true;
    return;
  }
  s_var *new = xmalloc(sizeof(*new));
  new->to_export = true;
  new->touched = true;
  new->value = NULL;
  htable_add(env->vars, name, new);
}


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
  char *name = strtok_r(var, "=", &word);
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
    return 1;
  }
  if (remove)
    unexport_var(env, name);
  else if (*word == '\0' && *(word - 1) != '=')
    export_novalue(env, name);
  else
    export_value(env, name, expand(word, env, cont));
  regfree(&regex);
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
      else if (var->to_export)
        printf("export %s\n", pair->key);
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
