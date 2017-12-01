#pragma once

#include <stdbool.h>
#include <stdio.h>

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
#include "ast/until.h"
#include "ast/while.h"
#include "shexec/environment.h"


#define DECLARE_AST_TYPE_ENUM(Name, Printer, Exec)                       \
  Name,

#define DECLARE_AST_PRINT_UTILS(Name, Printer, Exec)                     \
  Printer,

#define DECLARE_AST_EXEC_UTILS(Name, Printer, Exec)                      \
  Exec,

#define AST_TYPE_APPLY(F)                                                \
  F(SHNODE_CMD, cmd_print, cmd_exec)                                     \
  F(SHNODE_IF, if_print, if_exec)                                        \
  F(SHNODE_FOR, for_print, for_exec)                                     \
  F(SHNODE_WHILE, while_print, while_exec)                               \
  F(SHNODE_UNTIL, until_print, until_exec)                               \
  F(SHNODE_REDIRECTION, redirection_print, NULL)                         \
  F(SHNODE_PIPE, pipe_print, NULL)                                       \
  F(SHNODE_CASE, case_print, NULL)                                       \
  F(SHNODE_BOOL_OP, bool_op_print, bool_op_exec)                         \
  F(SHNODE_LIST, list_print, NULL)                                       \
  F(SHNODE_ASSIGNMENT, assignment_print, NULL)                           \
  F(SHNODE_FUNCTION, function_print, NULL)                               \
  F(SHNODE_BLOCK, if_print, NULL)


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
  } data;
} s_ast;


#define AST(Type, Field, Data)                  \
  ((s_ast)                                      \
  {                                             \
    .type = (Type),                             \
    .data.ast_ ## Field = (Data),               \
  })



void ast_print_rec(FILE *f, s_ast *ast);
void ast_print(FILE *f, s_ast *ast);
int ast_exec(s_env *env, s_ast *ast);
