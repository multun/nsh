#pragma once

#include "wordlist.h"


typedef struct acase_node
{
  s_wordlist *pattern;
  struct ast *action;
  struct acase_node *next;
} s_acase_node;


typedef struct acase
{
  s_wordlist *var;
  s_acase_node *nodes;
} s_acase;
