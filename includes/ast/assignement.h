#pragma once

#include "wordlist.h"

typedef struct aassignement
{
  s_wordlist *name;
  struct ast *value;
} s_aassignement;


#define AASSIGNEMENT(Name, Value)               \
  ((s_aassignement)                             \
  {                                             \
    (Name), (Value)                             \
  })
