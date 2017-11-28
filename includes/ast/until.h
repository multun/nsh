#pragma once

typedef struct auntil
{
  struct ast *condition;
  struct ast *actions;
} s_auntil;


#define AUNTIL(Condition, Actions)                \
  ((s_auntil)                                     \
  {                                               \
    (Condition), (Actions)                        \
  })
