#pragma once

#include <stddef.h>

#define SHOPTS_APPLY(F)                                                                  \
    F(AST_PRINT, "ast_print")                                                            \
    F(DOTGLOB, "dotglob")                                                                \
    F(EXPAND_ALIASES, "expand_aliases")                                                  \
    F(EXTGLOB, "extglob")                                                                \
    F(NOCASEGLOB, "nocaseglob")                                                          \
    F(NULLGLOB, "nullglob")                                                              \
    F(SOURCEPATH, "sourcepath")                                                          \
    F(XPG_ECHO, "xpg_echo")

#define SHOPT_ENUMIZE(EName, Repr) SHOPT_##EName,

enum shopt
{
    SHOPTS_APPLY(SHOPT_ENUMIZE)
    // this guard counts the number
    // of elements in the enum
    SHOPT_COUNT,
};

extern int g_shopts[SHOPT_COUNT];

/**
** \brief returns the shopt enum value from its string representation
** \return SHOPT_COUNT on fail, the corresponding enum value otherwise
*/
enum shopt shopt_from_string(const char *str);

/**
** \brief returns the string representation of a shopt
** \return NULL on fail, the corresponding string representation otherwise
*/
const char *string_from_shopt(size_t index);
