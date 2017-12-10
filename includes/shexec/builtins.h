#pragma once


#define BUILTINS_APPLY(F)                       \
  F(cd)                                         \
  F(echo)                                       \
  F(shopt)

#define BUILTINS_DECLARE(Name)                  \
  int builtin_ ## Name (int argc, char **argv);

typedef int (*f_builtin)(int argc, char **argv);

BUILTINS_APPLY(BUILTINS_DECLARE)

f_builtin builtin_search(const char *name);
