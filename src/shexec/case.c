#include "shexp/expansion.h"
#include "shparse/ast.h"

#include <stdio.h>
#include <string.h>
#include <fnmatch.h>

void case_print(FILE *f, struct shast *ast)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    fprintf(f, "\"%p\" [label=\"CASE\"];\n", (void*)ast);
    for (size_t case_i = 0; case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        ast_print_rec(f, case_item->action);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"", (void*)ast, (void*)case_item->action);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            if (i > 0)
                fputc('|', f);
            fprintf(f, "%s", shword_buf(wordlist_get(&case_item->pattern, i)));
        }
        fprintf(f, "\"];\n");
    }
}

int case_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    char *case_var = expand(&case_node->base.line_info, shword_buf(case_node->var), env, cont);
    for (size_t case_i = 0; case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            char *pattern = shword_buf(wordlist_get(&case_item->pattern, i));
            if (fnmatch(pattern, case_var, 0) != 0)
                continue;

            free(case_var);
            return ast_exec(env, case_item->action, cont);
        }
    }
    free(case_var);
    return 0;
}

static void case_item_free(struct shast_case_item *case_item)
{
    wordlist_destroy(&case_item->pattern);
    ast_free(case_item->action);
    free(case_item);
}

void case_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_case *case_node = (struct shast_case *)ast;

    free(case_node->var);
    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++)
        case_item_free(case_item_vect_get(&case_node->cases, case_i));
    case_item_vect_destroy(&case_node->cases);
    free(ast);
}
