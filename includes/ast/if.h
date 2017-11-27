#pragma once

#include "wordlist.h"


typedef struct aif
{
  struct ast *condition;
  struct ast *success;
  struct ast *failure;
} s_aif;
