#include <stdio.h>

#include "utils/macros.h"
#include "shparse/ast.h"
#include "utils/hash_table.h"

void function_print(FILE *f, struct shast *ast)
{
    struct shast_function *function = (struct shast_function *)ast;
    fprintf(f, "\"%p\" [label=\"FUNC\n%s\"];\n", (void*)ast, function->name);
    ast_print_rec(f, function->body);
    fprintf(f, "\"%p\" -> \"%p\";\n", (void*)ast, (void*)function->body);
}

int function_exec(struct environment *env, struct shast *ast, struct errcont *cont __unused)
{
    struct shast_function *function = (struct shast_function *)ast;
    char *name = function->name;
    struct pair *prev = htable_access(env->functions, name);
    if (prev)
        htable_remove(env->functions, name);
    htable_add(env->functions, name, function->body);
    return 0;
}

void function_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_function *func = (struct shast_function *)ast;
    ref_put(&func->refcnt);
}

void shast_function_ref_free(struct refcnt *refcnt)
{
    struct shast_function *func;
    func = container_of(refcnt, struct shast_function, refcnt);
    free(func->name);
    ast_free(func->body);
    free(func);
}
