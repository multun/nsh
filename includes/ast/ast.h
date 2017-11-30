#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "ast/bool_op.h"
#include "ast/cmd.h"
#include "ast/if.h"
#include "ast/for.h"
#include "ast/while.h"
#include "ast/until.h"
#include "ast/redirection.h"
#include "ast/pipe.h"
#include "ast/case.h"
#include "ast/bool_op.h"
#include "ast/list.h"
#include "ast/assignement.h"
#include "ast/function.h"
#include "ast/block.h"


#define DECLARE_AST_TYPE_ENUM(Name, Printer, Exec)                       \
  Name,

#define DECLARE_AST_PRINT_UTILS(Name, Printer, Exec)                     \
  Printer,

#define AST_TYPE_APPLY(F)                                                \
  F(SHNODE_CMD, cmd_print, NULL)                                         \
  F(SHNODE_IF, if_print, NULL)                                           \
  F(SHNODE_FOR, for_print, NULL)                                         \
  F(SHNODE_WHILE, while_print, NULL)                                     \
  F(SHNODE_UNTIL, until_print, NULL)                                     \
  F(SHNODE_REDIRECTION, redirection_print, NULL)                         \
  F(SHNODE_PIPE, pipe_print, NULL)                                             \
  F(SHNODE_CASE, case_print, NULL)                                       \
  F(SHNODE_BOOL_OP, bool_op_print, NULL)                                 \
  F(SHNODE_LIST, list_print, NULL)                                       \
  F(SHNODE_ASSIGNEMENT, assignement_print, NULL)                         \
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
    s_aassignement ast_assignement;
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
