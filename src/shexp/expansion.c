#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <string.h>

#include "shexec/builtin_shopt.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/evect.h"


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
    found = special_char_lookup(res, env, *var);
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

  bool num = false;
  if (is_name_char(**str, false))
    num = !is_name_char(**str, true);
  else
    return;

  evect_push(vec, **str);
  (*str)++;

  for (; **str && ((is_name_char(**str, false) && !num)
                   || (**str >= '0' && **str <= '9' && num))
         && (!braces || **str != '}'); (*str)++)
      evect_push(vec, **str);

  if (braces)
  {
    if (**str != '}')
      warnx("%s: bad substitution. expected '}'", *str);
    else
      (*str)++;
  }
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
    for (char *it = res; *it; it++)
    {
      if (expansion_protected_char(*it))
        evect_push(vec, '\\');
      evect_push(vec, *it);
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



static bool expand_dollar(s_exp_ctx ctx, s_evect *vec, s_env *env, s_errcont *cont)
{
  if (!(starts_expansion((*ctx.str)[1]) && (*ctx.str)++))
    return false;

  if (!ctx.quoted)
    evect_push(vec, '"');

  if (**ctx.str == '(' && (*ctx.str)++)
  {
    if (*(*ctx.str) == '(')
      expand_arth(ctx.str, env, vec, cont);
    else
      expand_subshell(cont, ctx.str, env, vec);
  }
  else
    expand_var(ctx.str, env, vec);

  if (!ctx.quoted)
    evect_push(vec, '"');

  return true;
}


bool expansion_protected_char(char c)
{
  switch (c)
  {
  case '\\': case '"':
    return true;
  default:
    return false;
  }
}

static void expand_sub(s_evect *vec, char *str, s_env *env, s_errcont *cont)
{
  bool sing_quote = false;
  bool doub_quote = false;
  s_exp_ctx ctx = EXPCTX(&str, &doub_quote);
  while (*str)
    if (!sing_quote && ((*str == '$' && expand_dollar(ctx, vec, env, cont))
                        || expand_backquote(cont, ctx, env, vec)))
      continue;
    else
    {
      if (*str == '\'' && !doub_quote)
        sing_quote = !sing_quote;
      else if (*str == '"' && !sing_quote)
        doub_quote = !doub_quote;
      else if (!sing_quote && *str == '\\')
      {
        evect_push(vec, *(str));
        str++;
      }
      evect_push(vec, *(str++));
    }
  evect_push(vec, '\0');
}


char *expand(char *str, s_env *env, s_errcont *cont)
{
  s_evect vec;
  evect_init(&vec, strlen(str) + 1);
  s_keeper keeper = KEEPER(cont->keeper);
  if (setjmp(keeper.env))
  {
    free(vec.data);
    shraise(cont, NULL);
  }
  else
    expand_sub(&vec, str, env, &ERRCONT(cont->errman, &keeper));
  return vec.data;
}
