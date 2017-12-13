#pragma once

#include "shexec/environment.h"
#include "utils/evect.h"


char *expand(char *str, s_env *env);
void expand_subshell(char **str, s_env *env, s_evect *vec);
void expand_arth(char **str, s_env *env, s_evect *vec);
