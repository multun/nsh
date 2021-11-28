#include <stdio.h>

#include <nsh_utils/macros.h>
#include <nsh_exec/ast_exec.h>
#include <nsh_utils/hashmap.h>


nsh_err_t function_exec(struct environment *env, struct shast *ast)
{
    struct shast_function *function = (struct shast_function *)ast;
    char *function_name = function->hash.key;

    // lookup the function
    struct hashmap_item **insertion_point;
    struct hashmap_item *func_hash =
        hashmap_find(&env->functions, &insertion_point, function_name);

    // if there's already a function there, remove it
    if (func_hash) {
        struct shast_function *former_func =
            container_of(func_hash, struct shast_function, hash);
        hashmap_remove(&env->functions, &former_func->hash);
        shast_ref_put(&former_func->base);
    }

    // insert the new function
    hashmap_insert(&env->functions, insertion_point, &function->hash);
    shast_ref_get(&function->base);
    return NSH_OK;
}
