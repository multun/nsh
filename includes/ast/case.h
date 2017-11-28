#pragma once

#include "wordlist.h"


typedef struct acase_node
{
  s_wordlist *pattern;
  struct ast *action;
  struct acase_node *next;
} s_acase_node;


#define ACASE_NODE(Pattern, Action, Next)       \
  ((s_acase_node)                               \
  {                                             \
    (Pattern), (Action), (Next)                 \
  })


typedef struct acase
{
  s_wordlist *var;
  s_acase_node *nodes;
} s_acase;


#define ACASE(Var, Nodes)                       \
  ((s_acase)                                    \
  {                                             \
    (Var), (Nodes)                              \
  })
