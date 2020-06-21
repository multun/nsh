#pragma once

#include "shexec/environment.h"
#include "utils/error.h"

/**
** \brief lists all available builtins
*/
#define BUILTINS_APPLY(F)                                                                \
    F(break)                                                                             \
    F(cd)                                                                                \
    F(continue)                                                                          \
    F(echo)                                                                              \
    F(exit)                                                                              \
    F(export)                                                                            \
    F(history)                                                                           \
    F(printf)                                                                            \
    F(shopt)                                                                             \
    F(source)

#define BUILTINS_DECLARE(Name)                                                           \
    int builtin_##Name(struct environment*env, struct errcont *cont, int argc, char **argv);

typedef int (*f_builtin)(struct environment*env, struct errcont *cont, int argc, char **argv);

BUILTINS_APPLY(BUILTINS_DECLARE)

/**
** \brief searches for a builtin
** \param name the name of the builtin to look for
** \return either a pointer to the builtin, or NULL if none was found
*/
f_builtin builtin_search(const char *name);
