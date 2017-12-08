#pragma once

#include "wordlist.h"
#include "utils/error.h"


typedef struct aredirection
{
  enum redir_type
  {
    REDIR_LESS,
    REDIR_DLESS,
    REDIR_GREAT,
    REDIR_DGREAT,
    REDIR_LESSAND,
    REDIR_GREATAND,
    REDIR_LESSDASH,
    REDIR_LESSGREAT,
    REDIR_CLOBBER,
  } type;
  int left;
  struct wordlist *right;
  struct ast *action;
} s_aredirection;


#define AREDIRECTION(Type, Left, Right, Action)                   \
  ((s_aredirection)                                               \
  {                                                               \
    (Type), (Left), (Right), (Action)                             \
  })

#define AST_AREDIRECTION(Type, Left, Right, Action)               \
  AST(SHNODE_REDIRECTION, redirection,                            \
      AREDIRECTION(Type, Left, Right, Action))


void redirection_print(FILE *f, struct ast *ast);
int redirection_exec(s_env *env, struct ast *ast,
                     struct ast *cmd, s_errcont *cont);
void redirection_free(struct ast *ast);
