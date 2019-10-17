#include "ast/ast_list.h"
#include "utils/alloc.h"

struct ast_list *ast_list_append(struct ast_list *list, struct ast *ast)
{
    struct ast_list *new = xmalloc(sizeof(struct ast_list));
    new->next = list;
    new->ast = ast;
    return new;
}

void ast_list_free(struct ast_list *list)
{
    while (list) {
        ast_free(list->ast);
        struct ast_list *tmp = list->next;
        free(list);
        list = tmp;
    }
}
