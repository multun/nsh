#pragma once

#include "wordlist.h"
#include "utils/error.h"

#define REDIRECTIONS_APPLY(F)                                                            \
    F(LESS, "<", redir_less)                                                             \
    F(DLESS, "<<", NULL)                                                                 \
    F(GREAT, ">", redir_great)                                                           \
    F(DGREAT, ">>", redir_dgreat)                                                        \
    F(LESSAND, "<&", redir_lessand)                                                      \
    F(GREATAND, ">&", redir_greatand)                                                    \
    F(LESSDASH, "<-", NULL)                                                              \
    F(LESSGREAT, "<>", redir_lessgreat)                                                  \
    F(CLOBBER, ">|", NULL)

#define REDIRECTIONS_ENUM(EName, Repr, Func) REDIR_##EName,

/**
** \brief represents a redirection node
*/
struct aredirection
{
    enum redir_type
    {
        REDIRECTIONS_APPLY(REDIRECTIONS_ENUM)
    } type; /**< the type of redirection */
    int left; /**< the io number */
    struct wordlist *right; /**< the command */
    struct ast *action; /**< the next redirection */
};

#define AREDIRECTION(Type, Left, Right, Action)                                          \
    ((struct aredirection){(Type), (Left), (Right), (Action)})

#define AST_AREDIRECTION(Type, Left, Right, Action)                                      \
    AST(SHNODE_REDIRECTION, redirection, AREDIRECTION(Type, Left, Right, Action))

/**
** \brief print in dot format the representation of a redirection node
*/
void redirection_print(FILE *f, struct ast *ast);

/**
** \brief exec a redirection node
*/
int redirection_exec(struct environment*env, struct ast *ast, struct ast *cmd, struct errcont *cont);

/**
** \brief free a redirection node
*/
void redirection_free(struct ast *ast);
