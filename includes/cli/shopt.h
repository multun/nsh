#pragma once


#include <stdbool.h>


#define SHOPTS_APPLY(F)                         \
  F(AST_PRINT, "ast_print")                     \
  F(DOTGLOB, "dotglob")                         \
  F(EXPAND_ALIASES, "expand_aliases")           \
  F(EXTGLOB, "extglob")                         \
  F(NOCASEGLOB, "nocaseglob")                   \
  F(NULLGLOB, "nullglob")                       \
  F(SOURCEPATH, "sourcepath")                   \
  F(XPG_ECH, "xpg_ech")


typedef enum shopt
{
  SHOPT_AST_PRINT,
  SHOPT_DOTGLOB,
  SHOPT_EXPAND_ALIASES,
  SHOPT_EXTGLOB,
  SHOPT_NOCASEGLOB,
  SHOPT_NULLGLOB,
  SHOPT_SOURCEPATH,
  SHOPT_XPG_ECH,
  // this guard counts the number
  // of elements in the enum
  SHOPT_COUNT,
} e_shopt;


extern bool g_shopts[SHOPT_COUNT];


e_shopt shopt_from_string(const char *str);
