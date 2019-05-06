#include <stdio.h>

#include "ast/ast.h"
#include "shexec/environment.h"

void block_print(FILE *f, struct ast *node)
{
    s_ablock *ablock = &node->data.ast_block;
    void *id = node;
    fprintf(f, "\"%p\" [label=\"BLOCK\"];\n", id);
    if (ablock->redir) {
        ast_print_rec(f, ablock->redir);
        void *id_next = ablock->redir;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"REDIR\"];\n", id, id_next);
    }
    if (ablock->def) {
        ast_print_rec(f, ablock->def);
        void *id_next = ablock->def;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"DEF\"];\n", id, id_next);
    }
    if (ablock->cmd) {
        ast_print_rec(f, ablock->cmd);
        void *id_next = ablock->cmd;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"CMD\"];\n", id, id_next);
    }
}

int block_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
    s_ablock *ablock = &ast->data.ast_block;

    if (ablock->redir) {
        s_ast *redir = ablock->redir;
        ablock->redir = NULL;
        int res = redirection_exec(env, redir, ast, cont);
        ablock->redir = redir;
        return res;
    }
    if (ablock->def)
        return assignment_exec(env, ablock->def, ablock->cmd, cont);
    if (ablock->cmd)
        return ast_exec(env, ablock->cmd, cont);

    return 0;
}

void block_free(struct ast *ast)
{
    if (!ast)
        return;
    ast_free(ast->data.ast_block.redir);
    ast_free(ast->data.ast_block.def);
    ast_free(ast->data.ast_block.cmd);
    free(ast);
}
