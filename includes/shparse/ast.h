#pragma once

#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/alloc.h"
#include "utils/error.h"
#include "utils/lineinfo.h"
#include "wordlist.h"
#include "utils/refcnt.h"
#include "utils/macros.h"

#include <stdbool.h>
#include <stdio.h>

#define DECLARE_AST_TYPE_ENUM(EnumName, Name) EnumName,

#define AST_TYPE_APPLY(F)                   \
    F(SHNODE_CMD, cmd)                      \
    F(SHNODE_IF, if)                        \
    F(SHNODE_FOR, for)                      \
    F(SHNODE_WHILE, while)                  \
    F(SHNODE_PIPE, pipe)                    \
    F(SHNODE_CASE, case)                    \
    F(SHNODE_BOOL_OP, bool_op)              \
    F(SHNODE_LIST, list)                    \
    F(SHNODE_SUBSHELL, subshell)            \
    F(SHNODE_FUNCTION, function)            \
    F(SHNODE_BLOCK, block)

/**
** \brief represent an Abstract Syntax Tree (AST).
**/
struct shast
{
    enum shnode_type
    {
        AST_TYPE_APPLY(DECLARE_AST_TYPE_ENUM)
    } type; /**< type of node */

    struct lineinfo line_info;
};

#define DECLARE_AST_PRINT_UTILS(EnumName, Name) \
    void Name ## _print(FILE *f, struct shast *ast);

#define DECLARE_AST_EXEC_UTILS(EnumName, Name) \
    int Name ## _exec(struct environment *env, struct shast *ast, struct errcont *cont);

#define DECLARE_AST_FREE_UTILS(EnumName, Name) \
    void Name ## _free(struct shast *ast);

AST_TYPE_APPLY(DECLARE_AST_PRINT_UTILS)
AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)
AST_TYPE_APPLY(DECLARE_AST_FREE_UTILS)

#define GVECT_NAME shast_vect
#define GVECT_TYPE struct shast *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE


static inline void shast_init(struct shast *ast, enum shnode_type type, struct lexer *lexer)
{
    ast->type = type;
    ast->line_info = *lexer_line_info(lexer);
}

#define DEFINE_AST_TYPE(Name, TypeVal)                                  \
    static inline struct Name *Name##_create(struct lexer *lexer)       \
    {                                                                   \
        struct Name *res = xcalloc(sizeof(*res), 1);                    \
        Name##_init(lexer, res);                                        \
        return res;                                                     \
    }                                                                   \
                                                                        \
    static inline struct Name *Name##_attach(struct shast **slot,       \
                                             struct lexer *lexer)       \
    {                                                                   \
        struct Name *res = Name##_create(lexer);                        \
        if (slot)                                                       \
            *slot = &res->base;                                         \
        return res;                                                     \
    }

/**
** \brief call a print function depending on node type
** \param f the file where to write
** \param ast the tree
**/
void ast_print_rec(FILE *f, struct shast *ast);

/**
** \brief print then whole tree in dot fromat
** \param f the file where to write
** \param ast the tree
**/
void ast_print(FILE *f, struct shast *ast);

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
int ast_exec(struct environment*env, struct shast *ast, struct errcont *cont);

/**
** \brief free ast recursively
** \param ast the tree
**/
void ast_free(struct shast *ast);


#define REDIRECTIONS_APPLY(F)                       \
    F(REDIR_LESS, "<", redir_less)                  \
    F(REDIR_DLESS, "<<", NULL)                      \
    F(REDIR_GREAT, ">", redir_great)                \
    F(REDIR_DGREAT, ">>", redir_dgreat)             \
    F(REDIR_LESSAND, "<&", redir_lessand)           \
    F(REDIR_GREATAND, ">&", redir_greatand)         \
    F(REDIR_LESSDASH, "<-", NULL)                   \
    F(REDIR_LESSGREAT, "<>", redir_lessgreat)       \
    F(REDIR_CLOBBER, ">|", NULL)


struct shast_redirection
{
    enum redir_type
    {
        REDIR_NONE = 0,
#define REDIRECTIONS_ENUM(EName, Repr, Func) EName,
        REDIRECTIONS_APPLY(REDIRECTIONS_ENUM)
#undef REDIRECTIONS_ENUM
    } type; /**< the type of redirection */
    int left; /**< the io number */
    char *right; /**< the redirection destination */
};

#define GVECT_NAME redir_vect
#define GVECT_TYPE struct shast_redirection *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE

void redir_vect_print(FILE *f, struct redir_vect *vect, void *id);

static inline void shast_redirection_free(struct shast_redirection *redir)
{
    free(redir->right);
    free(redir);
}

struct shast_assignment
{
    struct lineinfo line_info;
    char *name; /**< the variable name */
    char *value; /**< the value of the variable */
};

#define GVECT_NAME assign_vect
#define GVECT_TYPE struct shast_assignment *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE

void assign_vect_print(FILE *f, struct assign_vect *vect, void *parent);
void assignment_exec(struct environment *env, struct shast_assignment *ast, struct errcont *cont);

static inline void shast_assignment_free(struct shast_assignment *assign)
{
    free(assign->name);
    // the value is in the same string as the name
    free(assign);
}

struct shast_block
{
    struct shast base;
    struct redir_vect redirs; /**< the redirections */
    struct assign_vect assigns; /**< assignments */
    struct shast *command; /**< the command node */
};


static inline void shast_block_init(struct lexer *lexer,
                                    struct shast_block *node)
{
    shast_init(&node->base, SHNODE_BLOCK, lexer);
    redir_vect_init(&node->redirs, 4);
    assign_vect_init(&node->assigns, 4);
}

DEFINE_AST_TYPE(shast_block, SHNODE_BLOCK)

struct shast_bool_op
{
    struct shast base;
    enum bool_type
    {
        BOOL_OR,
        BOOL_AND,
        BOOL_NOT,
    } type; /**< the type of operator */
    struct shast *left; /**< the first operand */
    struct shast *right; /**< the second operand */
};

static inline void shast_bool_op_init(struct lexer *lexer,
                                      struct shast_bool_op *node)
{
    shast_init(&node->base, SHNODE_BOOL_OP, lexer);
}

DEFINE_AST_TYPE(shast_bool_op, SHNODE_BOOL_OP)

struct shast_case_item
{
    struct wordlist pattern; /**< the current tested value */
    struct shast *action; /**< the command to execute in case of match */
};

static inline void shast_case_item_init(struct shast_case_item *node)
{
    wordlist_init(&node->pattern);
}

#define GVECT_NAME case_item_vect
#define GVECT_TYPE struct shast_case_item *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE

struct shast_case
{
    struct shast base;
    char *var; /**< the tested variable */
    struct case_item_vect cases; /**< the case items */
};

static inline void shast_case_init(struct lexer *lexer, struct shast_case *node)
{
    shast_init(&node->base, SHNODE_CASE, lexer);
    case_item_vect_init(&node->cases, 4);
}

DEFINE_AST_TYPE(shast_case, SHNODE_CASE)

struct shast_cmd
{
    struct shast base;
    struct wordlist arguments;
};

static inline void shast_cmd_init(struct lexer *lexer, struct shast_cmd *node)
{
    shast_init(&node->base, SHNODE_CMD, lexer);
    // commands get a 4 arguments vector per default
    wordlist_init(&node->arguments);
}

DEFINE_AST_TYPE(shast_cmd, SHNODE_CMD)

struct shast_for
{
    struct shast base;
    char *var; /**< the variable */
    struct wordlist collection; /**< the list of value */
    struct shast *body; /**< the action to execute for each value */
};

static inline void shast_for_init(struct lexer *lexer, struct shast_for *node)
{
    shast_init(&node->base, SHNODE_FOR, lexer);
    wordlist_init(&node->collection);
}

DEFINE_AST_TYPE(shast_for, SHNODE_FOR)

struct shast_function
{
    struct shast base;
    struct refcnt refcnt;
    struct hash_head hash;
    struct shast *body; /**< the function body */
};

void shast_function_ref_free(struct refcnt *refcnt);

static inline void shast_function_hash_put(struct hash_head *head)
{
    struct shast_function *func = container_of(head, struct shast_function, hash);
    ref_put(&func->refcnt);
}

static inline void shast_function_init(struct lexer *lexer, struct shast_function *node)
{
    shast_init(&node->base, SHNODE_FUNCTION, lexer);
    ref_init(&node->refcnt, shast_function_ref_free);
}

DEFINE_AST_TYPE(shast_function, SHNODE_FUNCTION)

struct shast_if
{
    struct shast base;
    struct shast *condition; /**< the if condition */
    struct shast *branch_true; /**< the command to execute in case of success*/
    struct shast *branch_false; /**< the command to execute in case of failure*/
};

static inline void shast_if_init(struct lexer *lexer, struct shast_if *node)
{
    shast_init(&node->base, SHNODE_IF, lexer);
}

DEFINE_AST_TYPE(shast_if, SHNODE_IF)

struct shast_list
{
    struct shast base;
    struct shast_vect commands; /**< the list of commands to execute */
};

static inline void shast_list_init(struct lexer *lexer, struct shast_list *node)
{
    shast_init(&node->base, SHNODE_LIST, lexer);
    shast_vect_init(&node->commands, 4);
}

DEFINE_AST_TYPE(shast_list, SHNODE_LIST)

struct shast_pipe
{
    struct shast base;
    struct shast *left; /**< the left operand */
    struct shast *right; /**< the right operand */
};

static inline void shast_pipe_init(struct lexer *lexer, struct shast_pipe *node)
{
    shast_init(&node->base, SHNODE_PIPE, lexer);
}

DEFINE_AST_TYPE(shast_pipe, SHNODE_PIPE)

struct shast_subshell
{
    struct shast base;
    struct shast *action; /**< the subshell command */
};

static inline void shast_subshell_init(struct lexer *lexer, struct shast_subshell *node)
{
    shast_init(&node->base, SHNODE_SUBSHELL, lexer);
}

DEFINE_AST_TYPE(shast_subshell, SHNODE_SUBSHELL)

struct shast_while
{
    struct shast base;
    struct shast *condition; /**< the loop condition */
    struct shast *body; /**< the actions to execute into the loop */
    bool is_until;
};

static inline void shast_while_init(struct lexer *lexer, struct shast_while *node)
{
    shast_init(&node->base, SHNODE_WHILE, lexer);
}

DEFINE_AST_TYPE(shast_while, SHNODE_WHILE)
