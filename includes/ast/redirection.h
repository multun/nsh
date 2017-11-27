#pragma once


typedef struct aredirection
{
  enum type
  {
    IN,
    OUT,
    IN_CONC,
    OUT_CONC,
    // TODO
  } type;
  int left;
  wordlist *right;
  struct ast *action;
} s_aredirection;
