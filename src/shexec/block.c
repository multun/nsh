#include <stdio.h>

#include "ast/ast.h"
#include "shexec/environment.h"


void block_print(FILE *f, struct ast *node)
{
  s_ablock *ablock = &node->data.ast_block;
  void *id = node;
  fprintf(f, "\"%p\" [label=\"BLOCK\"];\n", id);
  if (ablock->redir)
  {
    ast_print_rec(f, ablock->redir);
    void *id_next = ablock->redir;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"REDIR\"];\n", id, id_next);
  }
  if (ablock->def)
  {
    ast_print_rec(f, ablock->def);
    void *id_next = ablock->def;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"DEF\"];\n", id, id_next);
  }
  if (ablock->cmd)
  {
    ast_print_rec(f, ablock->cmd);
    void *id_next = ablock->cmd;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"CMD\"];\n", id, id_next);
  }
}


int block_exec(s_env *env, s_ast *ast)
{
  s_ablock *ablock = &ast->data.ast_block;

  if (ablock->redir)
    ast_exec(env, ablock->redir);
  if (ablock->def)
    ast_exec(env, ablock->def);
  if (ablock->cmd)
    return ast_exec(env, ablock->cmd);

  return 0;
}
