#pragma once

#include "wordlist.h"


typedef struct afunction
{
  s_wordlist *name;
  struct ast *value;
} s_afunction;


#define AFUNCTION(Name, Value)                                                \
  ((s_afunction)                                                              \
  {                                                                           \
    (Name), (Value)                                                           \
  })

#define AST_AFUNCTION(Name, Value)                                            \
  AST(SHNODE_FUNCTION, function, AFUNCTION(Name, Value)


void function_print(FILE *f, struct ast *ast);
void function_free(struct ast *ast);
int function_exec(s_env *env, struct ast *ast);
