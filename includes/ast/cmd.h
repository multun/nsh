#pragma once

#include "shexec/environment.h"
#include "wordlist.h"
#include "utils/error.h"
#include "utils/pvect.h"
#include "utils/alloc.h"

struct ast;

/**
** \brief represents a command node
*/
struct acmd
{
    struct wordlist commands;
};

static inline void acmd_init(struct acmd *node)
{
    // commands get a 4 arguments vector per default
    wordlist_init(&node->commands);
}

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
