#include <stdio.h>

#include "ast/ast.h"

static void (*ast_print_utils[])(FILE *f,
                                 struct ast *ast) = {AST_TYPE_APPLY(DECLARE_AST_PRINT_UTILS)};

void ast_print_rec(FILE *f, struct ast *ast)
{
    if (ast)
        ast_print_utils[ast->type](f, ast);
}

void ast_print(FILE *f, struct ast *ast)
{
    fprintf(f, "digraph G {\n");
    ast_print_rec(f, ast);
    fprintf(f, "}\n");
}
