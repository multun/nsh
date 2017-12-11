#include <string.h>

#include "shexec/args.h"
#include "shexec/environment.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"
#include "shexp/variable.h"


s_env *environment_create(char *argv[])
{
  s_env *env = xmalloc(sizeof (s_env));
  env->argv = argv_dup(argv);
  env->vars = htable_create(10);
  env->functions = htable_create(10);

  env->code = 0;

  env->break_count = 0;
  env->depth = 0;
  return env;
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

  htable_map(env->vars, var_free);
  argv_free(env->argv);
  htable_free(env->vars);
  htable_free(env->functions);
  free(env);
}
