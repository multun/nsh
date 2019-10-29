#include <stdio.h>

#include "shparse/ast.h"


int list_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_list *list = (struct shast_list *)ast;
    int res = 0;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        res = ast_exec(env, shast_vect_get(&list->commands, i), cont);
    return res;
}
