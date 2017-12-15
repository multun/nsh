#pragma once

#include "shexec/environment.h"
#include "utils/error.h"


#define BUILTINS_APPLY(F)                       \
  F(cd)                                         \
  F(echo)                                       \
  F(exit)                                       \
  F(break)                                      \
  F(continue)                                   \
  F(shopt)                                      \
  F(source)                                     \
  F(history)                                    \
  F(export)


#define BUILTINS_DECLARE(Name)                  \
  int builtin_ ## Name (s_env *env, s_errcont *cont, int argc, char **argv);

typedef int (*f_builtin)(s_env *env, s_errcont *cont, int argc, char **argv);

BUILTINS_APPLY(BUILTINS_DECLARE)

f_builtin builtin_search(const char *name);
void update_pwd(bool oldpwd, s_env *env);
void expand_shopt(char **res);
