#pragma once

#include "wordlist.h"

typedef struct aassignment
{
  s_wordlist *name;
  s_wordlist *value;
  struct ast *action;
} s_aassignment;


#define AASSIGNMENT(Name, Value, Action)                                     \
  ((s_aassignement)                                                           \
  {                                                                           \
    .name = Name,                                                             \
    .value = Value,                                                           \
    .action = Action,                                                         \
  })

#define AST_AASSIGNMENT(Name, Value, Action)                                 \
  AST(SHNODE_ASSIGNMENT, assignment, AASSIGNMENT(Name, Value, Action))


void assignment_print(FILE *f, struct ast *ast);
