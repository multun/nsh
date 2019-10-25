#include <stdio.h>

#include "shparse/ast.h"

#define AST_PRINT_UTILS(EnumName, Name) \
    [EnumName] = Name ## _print,
static void (*ast_print_utils[])(FILE *f, struct shast *ast) =
{
    AST_TYPE_APPLY(AST_PRINT_UTILS)
};

void ast_print_rec(FILE *f, struct shast *ast)
{
    if (ast)
        ast_print_utils[ast->type](f, ast);
}

void ast_print(FILE *f, struct shast *ast)
{
    fprintf(f, "digraph G {\n");
    ast_print_rec(f, ast);
    fprintf(f, "}\n");
}
