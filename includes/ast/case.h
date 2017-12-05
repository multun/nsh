#pragma once

#include "wordlist.h"


typedef struct acase_node
{
  s_wordlist *pattern;
  struct ast *action;
  struct acase_node *next;
} s_acase_node;


#define ACASE_NODE(Pattern, Action, Next)                           \
  ((s_acase_node)                                                   \
  {                                                                 \
    (Pattern), (Action), (Next)                                     \
  })


typedef struct acase
{
  s_wordlist *var;
  s_acase_node *nodes;
} s_acase;


#define ACASE(Var, Nodes)                                           \
  ((s_acase)                                                        \
  {                                                                 \
    (Var), (Nodes)                                                  \
  })

#define AST_ACAST(Var, Nodes)                                       \
  AST(SHNODE_CASE, case, ACASE(Var, Nodes))


void case_print(FILE *f, struct ast *ast);
int case_exec(s_env *env, struct ast *ast);
void case_free(struct ast *ast);
