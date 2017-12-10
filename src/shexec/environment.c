#include "utils/alloc.h"

#include "shexec/environment.h"
#include "utils/hash_table.h"


s_env *environment_create()
{
  s_env *env = xmalloc(sizeof (s_env));
  env->vars = htable_create(10);
  env->functions = htable_create(10);
  return env;
}


static void var_free(struct pair *p)
{
  free(p->key);
  free(p->value);
}

void environment_free(s_env *env)
{
  htable_map(env->vars, var_free);
  htable_free(env->vars);
  htable_free(env->functions);
  free(env);
}
