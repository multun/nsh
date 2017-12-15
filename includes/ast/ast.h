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
#include "utils/error.h"
#include <stdbool.h>
#include <stdio.h>


#define DECLARE_AST_TYPE_ENUM(Name, Printer, Exec, Free)                      \
  Name,

#define DECLARE_AST_PRINT_UTILS(Name, Printer, Exec, Free)                    \
  Printer,

#define DECLARE_AST_EXEC_UTILS(Name, Printer, Exec, Free)                     \
  Exec,

#define DECLARE_AST_FREE_UTILS(Name, Printer, Exec, Free)                     \
  Free,

#define AST_TYPE_APPLY(F)                                                     \
  F(SHNODE_CMD, cmd_print, cmd_exec, cmd_free)                                \
  F(SHNODE_IF, if_print, if_exec, if_free)                                    \
  F(SHNODE_FOR, for_print, for_exec, for_free)                                \
  F(SHNODE_WHILE, while_print, while_exec, while_free)                        \
  F(SHNODE_UNTIL, until_print, until_exec, until_free)                        \
  F(SHNODE_REDIRECTION, redirection_print, NULL, redirection_free)            \
  F(SHNODE_PIPE, pipe_print, pipe_exec, pipe_free)                            \
  F(SHNODE_CASE, case_print, case_exec, case_free)                            \
  F(SHNODE_BOOL_OP, bool_op_print, bool_op_exec, bool_op_free)                \
  F(SHNODE_LIST, list_print, list_exec, list_free)                            \
  F(SHNODE_SUBSHELL, subshell_print, subshell_exec, subshell_free)            \
  F(SHNODE_ASSIGNMENT, assignment_print, NULL, assignment_free)               \
  F(SHNODE_FUNCTION, function_print, function_exec, function_free)            \
  F(SHNODE_BLOCK, block_print, block_exec, block_free)


/**
** \brief Abstract Syntax Tree
**/
typedef struct ast
{
  enum shnode_type
  {
    AST_TYPE_APPLY(DECLARE_AST_TYPE_ENUM)
  } type;

  union
  {
    s_acmd ast_cmd;
    s_aif ast_if;
    s_afor ast_for;
    s_awhile ast_while;
    s_auntil ast_until;
    s_aredirection ast_redirection;
    s_apipe ast_pipe;
    s_acase ast_case;
    s_abool_op ast_bool_op;
    s_alist ast_list;
    s_aassignment ast_assignment;
    s_afunction ast_function;
    s_ablock ast_block;
    s_asubshell ast_subshell;
  } data;
} s_ast;


#define AST(Type, Field, Data)                  \
  ((s_ast)                                      \
  {                                             \
    .type = (Type),                             \
    .data.ast_ ## Field = (Data),               \
  })


/**
** \brief call a print function depending on node type
** \param f the file where to write
** \param ast the tree
**/
void ast_print_rec(FILE *f, s_ast *ast);

/**
** \brief print then whole tree in dot fromat
** \param f the file where to write
** \param ast the tree
**/
void ast_print(FILE *f, s_ast *ast);

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
int ast_exec(s_env *env, s_ast *ast, s_errcont *cont);

/**
** \brief free ast recursively
** \param the tree
**/
void ast_free(s_ast *ast);
