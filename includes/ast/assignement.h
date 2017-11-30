#pragma once

#include "wordlist.h"

typedef struct aassignement
{
  s_wordlist *name;
  s_wordlist *value;
  struct ast *action;
} s_aassignement;


#define AASSIGNEMENT(Name, Value, Action)                                     \
  ((s_aassignement)                                                           \
  {                                                                           \
    .name = Name                                                              \
    .value = Value                                                            \
    .action = Action                                                          \
  })

#define AST_AASSIGNEMENT(Name, Value, Action)                                 \
  AST(SHNODE_ASSIGNEMENT, assignement, AASSIGNEMENT(Name, Value, Action))


void assignement_print(FILE *f, struct ast *ast);
