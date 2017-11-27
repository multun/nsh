#pragma once


typedef struct alist
{
  struct ast *action;
  struct alist *next;
} s_alist;
