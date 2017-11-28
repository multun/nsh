#pragma once

#include "wordlist.h"

typedef struct aredirection
{
  enum redir_type
  {
    REDIR_IN,
    REDIR_OUT,
    REDIR_AOUT,
    // TODO
  } type;
  int left;
  struct wordlist *right;
  struct ast *action;
} s_aredirection;
