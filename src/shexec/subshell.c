#include "ast/ast.h"
#include "utils/hash_table.h"
#include "shexec/clean_exit.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void subshell_print(FILE *f, s_ast *ast)
{
  s_asubshell *subshell = &ast->data.ast_subshell;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"SUBSHELL\"];\n", id);
  void *id_next = subshell->action;
  ast_print_rec(f, subshell->action);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}


void subshell_free(struct ast *ast)
{
  if (!ast)
    return;
  ast_free(ast->data.ast_subshell.action);
  free(ast);
}


int subshell_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  // TODO: error handling
  s_asubshell *asub = &ast->data.ast_subshell;

  int cpid = fork();
  if (cpid == 0)
    clean_exit(cont, ast_exec(env, asub->action, cont));

  int status;
  waitpid(cpid, &status, 0);
  return WEXITSTATUS(status);
}
