#pragma once


typedef struct awhile
{
  struct ast *condition;
  struct ast *actions;
} s_awhile;


#define AWHILE(Condition, Actions)                        \
  ((s_awhile)                                             \
  {                                                       \
    (Condition), (Actions)                                \
  })

#define AST_AWHILE(Condition, Actions)                    \
  AST(SHNODE_WHILE, while, AWHILE(Condition, Actions))
