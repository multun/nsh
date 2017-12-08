#include <err.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "shexec/clean_exit.h"


void pipe_print(FILE *f, s_ast *ast)
{
  s_apipe *apipe = &ast->data.ast_pipe;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"|\"];\n", id);

  void *id_left = apipe->left;
  ast_print_rec(f, apipe->left);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

  void *id_right = apipe->right;
  ast_print_rec(f, apipe->right);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
}


void pipe_free(struct ast *ast)
{
  if (!ast)
    return;
  ast_free(ast->data.ast_pipe.left);
  ast_free(ast->data.ast_pipe.right);
  free(ast);
}



int pipe_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_apipe *apipe = &ast->data.ast_pipe;

  int pd[2];
  if (pipe(pd) < 0)
  {
    warnx("42sh: pipe_exec: failed to pipe.");
    return 1;
  }
  int status1;
  int status2;
  pid_t pid1 = fork();
  pid_t pid2 = 1;
  if (pid1 > 0)
    pid2 = fork();

  if (pid1 < 0 || pid2 < 0)
    err(1, "42sh: pipe_exec: error while forking");

  else if (pid1 == 0)
  {
    if (close(pd[0]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[0]);

    if (dup2(pd[1], STDOUT_FILENO) < 0)
      errx(1, "42sh: pipe_exec: failed dup of %d", pd[1]);
    if (close(pd[1]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[1]);
    clean_exit(cont, ast_exec(env, apipe->left, cont));
  }

  else if (pid2 == 0)
  {
    if (close(pd[1]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[1]);

    if (dup2(pd[0], STDIN_FILENO) < 0)
      errx(1, "42sh: pipe_exec: failed dup of %d", pd[0]);
    if (close(pd[0]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[0]);
    clean_exit(cont, ast_exec(env, apipe->right, cont));
  }

  else
  {
    if (close(pd[0]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[0]);
    if (close(pd[1]) < 0)
      errx(1, "42sh: pipe_exec: error while closing %d", pd[1]);
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);
    return WEXITSTATUS(status2);
  }
  return 0;
}
