#pragma once

#include "wordlist.h"


typedef struct acmd
{
  s_wordlist *wordlist;
} s_acmd;


#define ACMD(Wordlist)                          \
  ((s_acmd)                                     \
  {                                             \
    (Wordlist)                                  \
  })

#define AST_ACMD(Wordlist)                           \
  AST(SHNODE_CMD, cmd, ACMD(Wordlist))


void cmd_print(FILE *f, struct ast *node);
int cmd_exec(struct ast *node);
