#include <stdio.h>

#include "shparse/ast.h"
#include "shexec/environment.h"

void bool_op_print(FILE *f, struct shast *ast)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    void *id = ast;
    if (bool_op->type == BOOL_OR)
        fprintf(f, "\"%p\" [label=\"OR\"];\n", id);
    else if (bool_op->type == BOOL_AND)
        fprintf(f, "\"%p\" [label=\"AND\"];\n", id);
    else
        fprintf(f, "\"%p\" [label=\"NOT\"];\n", id);

    ast_print_rec(f, bool_op->left);
    void *id_left = bool_op->left;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

    if (bool_op->type != BOOL_NOT) {
        ast_print_rec(f, bool_op->right);
        void *id_right = bool_op->right;
        fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
    }
}

int bool_op_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    if (bool_op->type == BOOL_AND) {
        int left = ast_exec(env, bool_op->left, cont);
        if (left)
            return left;
        return ast_exec(env, bool_op->right, cont);
    }
    if (bool_op->type == BOOL_OR) {
        if (!ast_exec(env, bool_op->left, cont))
            return 0;
        return ast_exec(env, bool_op->right, cont);
    } else
        return !ast_exec(env, bool_op->left, cont);
}

void bool_op_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    ast_free(bool_op->left);
    ast_free(bool_op->right);
    free(bool_op);
}
