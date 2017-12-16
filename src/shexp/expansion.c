#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <string.h>

#include "utils/evect.h"
#include "shexec/environment.h"
#include "shexec/builtins.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"



static bool predefined_lookup(char **res, s_env *env, char *var)
{
  for (size_t i = 0; var[i]; i++)
    if (!isdigit(var[i]))
      return false;

  size_t iarg = atoi(var);
  if (!iarg)
    *res = env->progname;
  else
  {
    for (size_t i = 0; i < iarg; i++)
      if (!env->argv[i])
      {
        *res = NULL;
        return false;
      }
    *res = env->argv[iarg];
  }

  free(var);
  return true;
}


static bool special_var_lookup(char **res, s_env *env, char *var)
{
  bool found = false;
  if (strlen(var) == 1)
    found = special_char_lookup(res, env, var);
  else if ((found = !strcmp("RANDOM", var)))
    expand_random(res);
  else if ((found = !strcmp("UID", var)))
    expand_uid(res);
  else if ((found = !strcmp("SHELLOPTS", var)))
    expand_shopt(res);
  if (found)
    free(var);
  return found;
}


static char *var_lookup(s_env *env, char *var)
{
  char *look = NULL;
  if (predefined_lookup(&look, env, var))
    return look;
  if (special_var_lookup(&look, env, var))
    return look;

  struct pair *var_pair = htable_access(env->vars, var);
  free(var);
  if (!var_pair)
    return NULL;
  s_var *nvar = var_pair->value;
  return nvar->value;
}


static bool is_name_char(char c, bool first)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'
         || (!first && c >= '0' && c <= '9');
}


static void fill_var(char **str, s_evect *vec, bool braces)
{

  evect_init(vec, strlen(*str));

  if (!is_name_char(**str, true))
    return;

  evect_push(vec, **str);
  (*str)++;

  for (; **str && ((!braces && is_name_char(**str, false)) || **str != '}'); (*str)++)
      evect_push(vec, **str);
}


static void expand_var(char **str, s_env *env, s_evect *vec)
{
  bool braces = **str == '{';
  if (braces)
    (*str)++;

  s_evect var;
  fill_var(str, &var, braces);

  size_t i = var.size;
  if (!braces)
    for (; i > 0; i--)
      if (var_lookup(env, strndup(var.data, i)))
        break;
  char *res = i ? var_lookup(env, strndup(var.data, i)) : NULL;
  evect_destroy(&var);

  if (res)
    for (; *res; res++)
      evect_push(vec, *res);
  if (braces)
  {
    assert(**str == '}');
    (*str)++;
  }
}


static bool starts_expansion(char c)
{
  if (isalpha(c) || isdigit(c))
    return true;

  switch (c)
  {
  case '(': case '{': case '?':
  case '@': case '*': case '$':
  case '#':
    return true;
  default:
    return false;
  }
}


static bool expand_dollar(char **str, s_evect *vec, s_env *env, s_errcont *cont)
{
  if (!(starts_expansion((*str)[1]) && (*str)++))
    return false;
  if (**str == '(' && (*str)++)
  {
    if (*(*str) == '(')
      expand_arth(str, env, vec);
    else
      expand_subshell(cont, str, env, vec);
  }
  else
    expand_var(str, env, vec);
  return true;
}


char *expand(char *str, s_env *env, s_errcont *cont)
{
  s_evect vec;
  evect_init(&vec, strlen(str) + 1);
  bool sing_quote = false;
  while (*str)
    if (!sing_quote && *str == '$' && expand_dollar(&str, &vec, env, cont))
      continue;
    else
    {
      if (*str == '\'')
        sing_quote = !sing_quote;
      else if (!sing_quote && *str == '\\')
        evect_push(&vec, *(str++));
      evect_push(&vec, *(str++));
    }
  evect_push(&vec, '\0');
  return vec.data;
}
