#include <stdio.h>

#include "shparse/ast.h"

void list_print(FILE *f, struct shast *ast)
{
    struct shast_list *list = (struct shast_list *)ast;

    void *id = ast;
    fprintf(f, "\"%p\" [label=\"LIST\"];\n", id);

    for (size_t i = 0; i < shast_vect_size(&list->commands); i++) {
        struct shast *cur = shast_vect_get(&list->commands, i);
        ast_print_rec(f, cur);
        void *id_cur = cur;
        fprintf(f, "\"%p\" -> \"%p\";\n", id, id_cur);
    }
}

int list_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_list *list = (struct shast_list *)ast;
    int res = 0;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        res = ast_exec(env, shast_vect_get(&list->commands, i), cont);
    return res;
}

void list_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_list *list = (struct shast_list *)ast;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        ast_free(shast_vect_get(&list->commands, i));
    shast_vect_destroy(&list->commands);
    free(ast);
}
