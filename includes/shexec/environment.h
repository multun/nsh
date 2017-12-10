#pragma once

#include "utils/hash_table.h"


typedef struct environment
{
  s_htable *vars;
  s_htable *functions;
  char **argv;
} s_env;


s_env *environment_create();
void environment_free(s_env *env);
