#pragma once

#include "shparse/ast.h"
#include "shexec/environment.h"

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
int ast_exec(struct environment*env, struct shast *ast, struct ex_scope *ex_scope);


#define DECLARE_AST_EXEC_UTILS(EnumName, Name) \
    int Name ## _exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope);

AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)

void assignment_exec(struct environment *env, struct shast_assignment *ast, struct ex_scope *ex_scope);


/**
** \brief generate from a wordlist a table of arguments
** \param wl the wordlist
** \param env the current environment
** \return argc
**/
void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                     struct environment *env, struct ex_scope *ex_scope);
