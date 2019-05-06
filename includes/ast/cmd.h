#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"

/**
** \brief represents a command node
*/
typedef struct acmd
{
    s_wordlist *wordlist; /**< the command to execute */
} s_acmd;

#define ACMD(Wordlist) ((s_acmd){(Wordlist)})

#define AST_ACMD(Wordlist) AST(SHNODE_CMD, cmd, ACMD(Wordlist))

/**
** \brief print in dot format the representation of a command node
*/
void cmd_print(FILE *f, struct ast *node);

/**
** \brief exec a command node
*/
int cmd_exec(s_env *env, struct ast *node, s_errcont *cont);

/**
** \brief free a command node
*/
void cmd_free(struct ast *ast);
