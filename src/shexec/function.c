#include <stdio.h>

#include "utils/macros.h"
#include "shparse/ast.h"
#include "utils/hash_table.h"

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
