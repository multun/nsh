#pragma once

#include "ast/assignment.h"
#include "ast/block.h"
#include "ast/bool_op.h"
#include "ast/bool_op.h"
#include "ast/case.h"
#include "ast/cmd.h"
#include "ast/for.h"
#include "ast/function.h"
#include "ast/if.h"
#include "ast/list.h"
#include "ast/pipe.h"
#include "ast/redirection.h"
#include "ast/subshell.h"
#include "ast/until.h"
#include "ast/while.h"
#include "shexec/environment.h"
#include "shlex/lexer.h"
#include "utils/alloc.h"
#include "utils/error.h"
#include "utils/lineinfo.h"

#include <stdbool.h>
#include <stdio.h>

#define DECLARE_AST_TYPE_ENUM(Name, Printer, Exec, Free) Name,

#define DECLARE_AST_PRINT_UTILS(Name, Printer, Exec, Free) Printer,

#define DECLARE_AST_EXEC_UTILS(Name, Printer, Exec, Free) Exec,

#define DECLARE_AST_FREE_UTILS(Name, Printer, Exec, Free) Free,

#define AST_TYPE_APPLY(F)                                                                \
    F(SHNODE_CMD, cmd_print, cmd_exec, cmd_free)                                         \
    F(SHNODE_IF, if_print, if_exec, if_free)                                             \
    F(SHNODE_FOR, for_print, for_exec, for_free)                                         \
    F(SHNODE_WHILE, while_print, while_exec, while_free)                                 \
    F(SHNODE_UNTIL, until_print, until_exec, until_free)                                 \
    F(SHNODE_REDIRECTION, redirection_print, NULL, redirection_free)                     \
    F(SHNODE_PIPE, pipe_print, pipe_exec, pipe_free)                                     \
    F(SHNODE_CASE, case_print, case_exec, case_free)                                     \
    F(SHNODE_BOOL_OP, bool_op_print, bool_op_exec, bool_op_free)                         \
    F(SHNODE_LIST, list_print, list_exec, list_free)                                     \
    F(SHNODE_SUBSHELL, subshell_print, subshell_exec, subshell_free)                     \
    F(SHNODE_ASSIGNMENT, assignment_print, NULL, assignment_free)                        \
    F(SHNODE_FUNCTION, function_print, function_exec, function_free)                     \
    F(SHNODE_BLOCK, block_print, block_exec, block_free)

/**
** \brief represent an Abstract Syntax Tree (AST).
**/
struct ast
{
    enum shnode_type
    {
        AST_TYPE_APPLY(DECLARE_AST_TYPE_ENUM)
    } type; /**< type of node */

    struct lineinfo line_info;

    union
    {
        struct acmd ast_cmd; /**< command field */
        struct aif ast_if; /**< if field */
        struct afor ast_for; /**< for field */
        struct awhile ast_while; /**< while field */
        struct auntil ast_until; /**< until field */
        struct aredirection ast_redirection; /**< redirection field */
        struct apipe ast_pipe; /**< pipe field */
        struct acase ast_case; /**< case field */
        struct abool_op ast_bool_op; /**< bool operator field */
        struct alist ast_list; /**< command field */
        struct aassignment ast_assignment; /**< assignment field */
        struct afunction ast_function; /**< function field */
        struct ablock ast_block; /**< block field */
        struct asubshell ast_subshell; /**< subshell field */
    } data; /**< content of the node */ /**< command field */
};

#define AST(Type, Field, Data)                                                           \
    ((struct ast){                                                                            \
        .type = (Type),                                                                  \
        .data.ast_##Field = (Data),                                                      \
    })

static inline struct ast *ast_create(enum shnode_type type, struct lexer *lexer)
{
    struct ast *res = xcalloc(sizeof(*res), 1);
    res->type = type;
    res->line_info = *lexer_line_info(lexer);
    return res;
}

/**
** \brief call a print function depending on node type
** \param f the file where to write
** \param ast the tree
**/
void ast_print_rec(FILE *f, struct ast *ast);

/**
** \brief print then whole tree in dot fromat
** \param f the file where to write
** \param ast the tree
**/
void ast_print(FILE *f, struct ast *ast);

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
int ast_exec(struct environment*env, struct ast *ast, struct errcont *cont);

/**
** \brief free ast recursively
** \param ast the tree
**/
void ast_free(struct ast *ast);
