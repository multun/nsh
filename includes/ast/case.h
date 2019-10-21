#pragma once

#include "wordlist.h"
#include "utils/error.h"

struct ast;

/**
** \brief represents a linked list of case
*/
struct acase_node
{
    struct wordlist pattern; /**< the current tested value */
    struct ast *action; /**< the command to execute in case of match */
    struct acase_node *next; /**< the next case */
};

static inline void acase_node_init(struct acase_node *node)
{
    wordlist_init(&node->pattern);
}

struct acase
{
    char *var; /**< the tested variable */
    struct acase_node *nodes; /**< the linked list of cases */
};

#define ACASE(Var, Nodes) ((struct acase){(Var), (Nodes)})

#define AST_ACAST(Var, Nodes) AST(SHNODE_CASE, case, ACASE(Var, Nodes))

/**
** \brief print in dot format the representation of a case node
*/
void case_print(FILE *f, struct ast *ast);

/**
** \brief exec a case node
*/
int case_exec(struct environment *env, struct ast *ast, struct errcont *cont);

/**
** \brief free a case node
*/
void case_free(struct ast *ast);
