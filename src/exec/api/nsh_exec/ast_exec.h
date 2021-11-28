#pragma once

#include <nsh_parse/ast.h>
#include <nsh_exec/environment.h>

/**
** \brief execute the tree
** \param env the current environment
** \param ast the tree
**/
nsh_err_t ast_exec(struct environment *env, struct shast *ast);


#define DECLARE_AST_EXEC_UTILS(EnumName, Name)                                           \
    int Name##_exec(struct environment *env, struct shast *ast);


AST_TYPE_APPLY(DECLARE_AST_EXEC_UTILS)
