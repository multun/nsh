#pragma once


typedef struct abool_op
{
  enum bool_type
  {
    BOOL_OR,
    BOOL_AND,
    BOOL_NOT,
  } type;
  struct ast *left;
  struct ast *right;
} s_abool_op;
