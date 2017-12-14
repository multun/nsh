#pragma once

#include "utils/hash_table.h"


typedef struct environment
{
  s_htable *vars;
  s_htable *functions;
  char **argv;

  size_t break_count;
  bool break_continue;
  size_t depth;

  int code;
} s_env;


s_env *environment_create();
void environment_load(s_env *env);
void environment_free(s_env *env);
