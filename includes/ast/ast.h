#pragma once

#include <stdbool.h>

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


typedef struct ast
{
  enum shnode_type
  {
    SHNODE_CMD,
    SHNODE_IF,
    SHNODE_FOR,
    SHNODE_WHILE,
    SHNODE_UNTIL,
    SHNODE_REDIRECTION,
    SHNODE_PIPE,
    SHNODE_CASE,
    SHNODE_BOOL_OP,
    SHNODE_LIST,
    SHNODE_ASSIGNEMENT,
    SHNODE_FUNCTION,
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
  } data;
} s_ast;
