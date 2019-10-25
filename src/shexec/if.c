#include <stdio.h>

#include "shparse/ast.h"
#include "shexec/environment.h"

void if_print(FILE *f, struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;

    // print the if label
    fprintf(f, "\"%p\" [label=\"IF\"];\n", (void*)ast);
    ast_print_rec(f, if_node->condition);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n",
            (void*)ast, (void*)if_node->condition);

    // print the true branch
    ast_print_rec(f, if_node->branch_true);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"THEN\"];\n",
            (void*)ast, (void*)if_node->branch_true);

    // print the false branch, if it exists
    if (if_node->branch_false) {
        ast_print_rec(f, if_node->branch_false);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"ELSE\"];\n",
                (void*)ast, (void*)if_node->branch_false);
    }
}

int if_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    int cond = ast_exec(env, if_node->condition, cont);
    if (!cond)
        return ast_exec(env, if_node->branch_true, cont);
    else if (if_node->branch_false)
        return ast_exec(env, if_node->branch_false, cont);
    return 0;
}

void if_free(struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    if (!ast)
        return;
    ast_free(if_node->condition);
    ast_free(if_node->branch_true);
    ast_free(if_node->branch_false);
    free(ast);
}
