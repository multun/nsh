#pragma once

#include "wordlist.h"
#include "utils/error.h"


/**
** \brief represents a redirection node
*/
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


/**
** \brief print in dot format the representation of a redirection node
*/
void redirection_print(FILE *f, struct ast *ast);


/**
** \brief exec a redirection node
*/
int redirection_exec(s_env *env, struct ast *ast,
                     struct ast *cmd, s_errcont *cont);


/**
** \brief free a redirection node
*/
void redirection_free(struct ast *ast);
