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
        struct wordlist *pattern = node->pattern;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"%s", id, id_next, pattern->str);
        pattern = pattern->next;
        while (pattern) {
            fprintf(f, "\n%s", pattern->str);
            pattern = pattern->next;
        }
        fprintf(f, "\"];\n");
        node = node->next;
    }
}

int case_exec(struct environment *env, struct ast *ast, struct errcont *cont)
{
    struct acase *acase = &ast->data.ast_case;
    struct acase_node *node = acase->nodes;

    while (node) {
        struct wordlist *pattern = node->pattern;
        while (pattern) {
            if (!strcmp(pattern->str, pattern->str))
                return ast_exec(env, node->action, cont);
            pattern = pattern->next;
        }
        node = node->next;
    }
    return 0;
}

static void case_node_free(struct acase_node *casen)
{
    if (!casen)
        return;
    wordlist_free(casen->pattern, true);
    ast_free(casen->action);
    case_node_free(casen->next);
    free(casen);
}

void case_free(struct ast *ast)
{
    if (!ast)
        return;
    wordlist_free(ast->data.ast_case.var, true);
    case_node_free(ast->data.ast_case.nodes);
    free(ast);
}
