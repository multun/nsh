#pragma once

typedef struct apipe
{
  struct ast *left;
  struct ast *right;
} s_apipe;


#define APIPE(Left, Right)                      \
  ((s_apipe)                                    \
  {                                             \
    (Left), (Right)                             \
  })
