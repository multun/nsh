#include <stdio.h>

#include "ast/ast.h"
#include "utils/hash_table.h"

void function_print(FILE *f, struct ast *ast)
{
    struct afunction *function = &ast->data.ast_function;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"FUNC\n%s\"];\n", id, function->name);
    void *id_next = function->value;
    ast_print_rec(f, function->value);
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}

void function_free(struct ast *ast)
{
    if (!ast)
        return;

    free(ast->data.ast_function.name);
    ast_free(ast->data.ast_function.value);
    free(ast);
}

int function_exec(struct environment *env, struct ast *ast, struct errcont *cont)
{
    if (!cont)
        abort();
    struct afunction *function = &ast->data.ast_function;
    char *name = function->name;
    struct pair *prev = htable_access(env->functions, name);
    if (prev)
        htable_remove(env->functions, name);
    htable_add(env->functions, name, function->value);
    return 0;
}
