#pragma once

#include <nsh_parse/ast.h>
#include <nsh_exec/environment.h>

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
int ast_exec(struct environment*env, struct shast *ast, struct ex_scope *ex_scope);


#define DECLARE_AST_EXEC_UTILS(EnumName, Name) \
    int Name ## _exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope);

AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)
