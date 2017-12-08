#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"


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
int cmd_exec(s_env *env, struct ast *node, s_errcont *cont);
void cmd_free(struct ast *ast);
