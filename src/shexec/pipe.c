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


enum pipe_pos
{
  PIPE_LEFT,
  PIPE_RIGHT,
};


int child_redir(int pd[2], enum pipe_pos pos)
{
  if (close(pd[pos]) < 0)
  {
    warnx("pipe_exec: error while closing %d", pd[pos]);
    return 1;
  }

  int target_fd = pos == PIPE_LEFT ? STDOUT_FILENO : STDIN_FILENO;
  if (dup2(pd[!pos], target_fd) < 0)
  {
    warnx("pipe_exec: failed dup of %d", pd[!pos]);
    return 1;
  }

  if (close(pd[!pos]) < 0)
  {
    warnx("pipe_exec: error while closing %d", pd[!pos]);
    return 1;
  }
  return 0;
}


struct pipe_context
{
  int pd[2];
  pid_t child_pid[2];
};


int pipe_father(struct pipe_context *pc)
{
  if (close(pc->pd[0]) < 0)
    warnx("pipe_exec: error while closing %d", pc->pd[0]);
  if (close(pc->pd[1]) < 0)
    warnx("pipe_exec: error while closing %d", pc->pd[1]);

  int status;
  waitpid(pc->child_pid[0], &status, 0);
  waitpid(pc->child_pid[1], &status, 0);
  return WEXITSTATUS(status);
}


bool pipe_init(struct pipe_context *pc)
{
  if (pipe(pc->pd) < 0)
  {
    warn("pipe_init: pipe failed");
    return true;
  }

  if ((pc->child_pid[0] = fork()) < 0
      || (pc->child_pid[0] > 0 && (pc->child_pid[1] = fork()) < 0))
  {
    warn("pipe_init: error while forking");
    return true;
  }

  return false;
}


int pipe_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_apipe *apipe = &ast->data.ast_pipe;
  struct pipe_context pc;
  if (pipe_init(&pc))
    return 1;

  else if (pc.child_pid[0] == 0)
  {
    int res = child_redir(pc.pd, PIPE_LEFT);
    clean_exit(cont, res ? res : ast_exec(env, apipe->left, cont));
  }
  else if (pc.child_pid[1] == 0)
  {
    int res = child_redir(pc.pd, PIPE_RIGHT);
    clean_exit(cont, res ? res : ast_exec(env, apipe->right, cont));
  }
  else
    return pipe_father(&pc);
}
