#pragma once

#include "wordlist.h"

typedef struct aassignement
{
  s_wordlist *name;
  s_wordlist *value;
  struct ast *action;
} s_aassignement;


#define AASSIGNEMENT(Name, Value, Action)       \
  ((s_aassignement)                             \
  {                                             \
    .name = Name                                \
    .value = Value                              \
    .action = Action                            \
  })
