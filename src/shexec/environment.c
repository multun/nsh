#include <string.h>

#include "shexec/args.h"
#include "shexec/environment.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"


s_env *environment_create(char *argv[])
{
  s_env *env = xmalloc(sizeof (s_env));
  env->argv = argv_dup(argv);
  env->vars = htable_create(10);
  env->functions = htable_create(10);
  env->code = 0;
  return env;
}


static void var_free(struct pair *p)
{
  free(p->key);
  free(p->value);
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
