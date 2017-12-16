#include <string.h>
#include <unistd.h>

#include "repl/repl.h"
#include "ast/assignment.h"
#include "ast/ast_list.h"
#include "shexec/args.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/variable.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"



static char **arg_context_extract(s_arg_context *args)
{
  int argc = args->argc - args->argc_base;
  // the first additional element is argv[0],
  // the other one is the terminating NULL
  char **ret = xcalloc(sizeof(char **), argc + 2);
  ret[0] = strdup(args->argv[args->progname_ind]);
  for (int i = 0; i < argc; i++)
    ret[i + 1] = strdup(args->argv[args->argc_base + i]);
  // no need to add the terminating null, as we already calloced
  return ret;
}

s_env *environment_create(s_arg_context *arg_cont)
{
  s_env *env = xmalloc(sizeof (s_env));
  env->argv = arg_context_extract(arg_cont);
  env->vars = htable_create(10);
  env->functions = htable_create(10);
  env->ast_list = NULL;
  env->code = 0;

  env->break_count = 0;
  env->depth = 0;
  environment_load(env);
  return env;
}


void environment_load(s_env *env)
{
  for (char **it = environ; *it; it++)
  {
    char *var = strdup(*it);
    char *save = NULL;
    char *name = strtok_r(var, "=", &save);
    char *value = strtok_r(NULL, "\0", &save);
    if (!value)
      value = xcalloc(1, sizeof(char));
    else
      value = strdup(value);
    assign_var(env, name, value, true);
    struct pair *p = htable_access(env->vars, name);
    s_var *node = p->value;
    node->to_export = true;
  }
  if (!htable_access(env->vars, "PWD"))
    update_pwd(false, env);
  if (!htable_access(env->vars, "IFS"))
    assign_var(env, strdup("IFS"), strdup("\t\n "), true);
}


static void var_free(struct pair *p)
{
  free(p->key);
  s_var *var = p->value;
  free(var->value);
  free(var);
}


void environment_free(s_env *env)
{
  if (!env)
    return;

  ast_list_free(env->ast_list);
  htable_map(env->vars, var_free);
  argv_free(env->argv);
  htable_free(env->vars);
  htable_free(env->functions);
  free(env);
}
