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


void environment_free(s_env *env)
{
  htable_clear(env->vars);
  free(env->vars);
  htable_clear(env->functions);
  free(env->functions);
  free(env);
}
