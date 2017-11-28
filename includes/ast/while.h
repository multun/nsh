#pragma once


typedef struct awhile
{
  struct ast *condition;
  struct ast *actions;
} s_awhile;


#define AWHILE(Condition, Actions)              \
  ((s_awhile)                                   \
  {                                             \
    (Condition), (Actions)                      \
  })
