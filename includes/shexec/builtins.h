#pragma once

#include "shexec/environment.h"
#include "utils/error.h"

/**
** \brief lists all available builtins
*/
#define BUILTINS_APPLY(F)                                                                \
    F(cd)                                                                                \
    F(echo)                                                                              \
    F(exit)                                                                              \
    F(break)                                                                             \
    F(continue)                                                                          \
    F(shopt)                                                                             \
    F(source)                                                                            \
    F(history)                                                                           \
    F(export)

#define BUILTINS_DECLARE(Name)                                                           \
    int builtin_##Name(s_env *env, s_errcont *cont, int argc, char **argv);

typedef int (*f_builtin)(s_env *env, s_errcont *cont, int argc, char **argv);

BUILTINS_APPLY(BUILTINS_DECLARE)

/**
** \brief searches for a builtin
** \param name the name of the builtin to look for
** \return either a pointer to the builtin, or NULL if none was found
*/
f_builtin builtin_search(const char *name);
