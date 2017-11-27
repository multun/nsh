#pragma once


typedef struct abool_op
{
  enum type
  {
    OR,
    AND,
    NOT,
  } type;
  struct ast *left;
  struct ast *right;
} s_abool_op;
