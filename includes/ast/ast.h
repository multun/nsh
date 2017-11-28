#pragma once

#include <stdbool.h>


typedef struct ast
{
  enum type
  {
    CMD,
    IF,
    FOR,
    WHILE,
    UNTIL,
    REDIRECTION,
    PIPE,
    CASE,
    BOOL_OP,
    LIST,
    ASSIGNEMENT,
    FUNCTION,
  } type;
  union
  {
    s_acmd *ast_cmd;
    s_aif *ast_if;
    s_afor *ast_for;
    s_awhile *ast_while;
    s_auntil *ast_until;
    s_aredirection *ast_redirection;
    s_apipe *ast_pipe;
    s_acase *ast_case;
    s_abool_op *ast_bool_op;
    s_alist *ast_list;
    s_aassignement *ast_assignement;
    s_afunction *ast_function;
  } data;
} s_ast;
