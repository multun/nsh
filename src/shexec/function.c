#include <stdio.h>

#include "utils/macros.h"
#include "shparse/ast.h"
#include "utils/hash_table.h"

void function_print(FILE *f, struct shast *ast)
{
    struct shast_function *function = (struct shast_function *)ast;
    fprintf(f, "\"%p\" [label=\"FUNC\n%s\"];\n", (void*)ast, hash_head_key(&function->hash));
    ast_print_rec(f, function->body);
    fprintf(f, "\"%p\" -> \"%p\";\n", (void*)ast, (void*)function->body);
}

int function_exec(struct environment *env, struct shast *ast, struct errcont *cont __unused)
{
    struct shast_function *function = (struct shast_function *)ast;
    char *function_name = hash_head_key(&function->hash);

    // lookup the function
    struct hash_head **insertion_point;
    struct hash_head *func_hash = hash_table_find(
        &env->functions, &insertion_point, function_name);

    // if there's already a function there, remove it
    if (func_hash)
    {
        struct shast_function *former_func = container_of(func_hash, struct shast_function, hash);
        hash_table_remove(&env->functions, &former_func->hash);
        ref_put(&former_func->refcnt);
    }

    // insert the new function
    hash_table_insert(&env->functions, insertion_point, &function->hash);
    ref_get(&function->refcnt);
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
    free(hash_head_key(&func->hash));
    ast_free(func->body);
    free(func);
}
