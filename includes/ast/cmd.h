#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"

struct ast;

/**
** \brief represents a command node
*/
struct acmd
{
    struct wordlist *wordlist; /**< the command to execute */
};

/**
** \brief print in dot format the representation of a command node
*/
void cmd_print(FILE *f, struct ast *node);

/**
** \brief exec a command node
*/
int cmd_exec(struct environment*env, struct ast *node, struct errcont *cont);

/**
** \brief free a command node
*/
void cmd_free(struct ast *ast);
