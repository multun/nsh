#include "ast/ast.h"

#include <stdio.h>
#include <string.h>

void case_print(FILE *f, struct ast *ast)
{
    struct acase *acase = &ast->data.ast_case;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"CASE\"];\n", id);
    struct acase_node *node = acase->nodes;

    while (node) {
        ast_print_rec(f, node->action);
        void *id_next = node->action;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"", id, id_next);
        for (size_t i = 0; i < wordlist_size(&node->pattern); i++)
        {
            if (i > 0)
                fputc('\n', f);
            fprintf(f, "%s", wordlist_get(&node->pattern, i));
        }
        fprintf(f, "\"];\n");
        node = node->next;
    }
}

int case_exec(struct environment *env, struct ast *ast, struct errcont *cont)
{
    struct acase *acase = &ast->data.ast_case;
    for (struct acase_node *node = acase->nodes; node; node = node->next) {
        for (size_t i = 0; i < wordlist_size(&node->pattern); i++) {
            char *pattern = wordlist_get(&node->pattern, i);
            // XXX: TODO: this is super broken. the case pattern isn't expanded
            if (strcmp(pattern, pattern) == 0)
                return ast_exec(env, node->action, cont);
        }
    }
    return 0;
}

static void case_node_free(struct acase_node *casen)
{
    if (!casen)
        return;

    wordlist_destroy(&casen->pattern);
    ast_free(casen->action);
    case_node_free(casen->next);
    free(casen);
}

void case_free(struct ast *ast)
{
    if (!ast)
        return;
    free(ast->data.ast_case.var);
    case_node_free(ast->data.ast_case.nodes);
    free(ast);
}
