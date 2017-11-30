#pragma once

typedef struct apipe
{
  struct ast *left;
  struct ast *right;
} s_apipe;


#define APIPE(Left, Right)                                        \
  ((s_apipe)                                                      \
  {                                                               \
    (Left), (Right)                                               \
  })

#define AST_PIPE(Left, Right)                                     \
  AST(SHNODE_PIPE, pipe, APIPE(Left, Right))
