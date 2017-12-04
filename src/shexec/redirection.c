#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ast/ast.h"


void redirection_print(FILE *f, s_ast *ast)
{
  s_aredirection *aredirection = &ast->data.ast_redirection;
  void *id = ast;
  char *redir = "<";

  if (aredirection->type == REDIR_DLESS)
    redir = "<<";
  else if (aredirection->type == REDIR_GREAT)
    redir = ">";
  else if (aredirection->type == REDIR_DGREAT)
    redir = ">>";
  else if (aredirection->type == REDIR_LESSAND)
    redir = "<&";
  else if (aredirection->type == REDIR_GREATAND)
    redir = "<&";
  else if (aredirection->type == REDIR_LESSDASH)
    redir = "<-";
  else if (aredirection->type == REDIR_LESSGREAT)
    redir = "<>";
  else if (aredirection->type == REDIR_CLOBBER)
    redir = ">|";

  fprintf(f, "\"%p\" [label=\"%d %s %s\"];\n", id, aredirection->left,
          redir, aredirection->right->str);

  if (aredirection->action)
  {
    ast_print_rec(f, aredirection->action);
    void *id_next = aredirection->action;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
  }
}


static int fd_save(int fd)
{
  int copy = dup(fd);
  if (copy < 0)
    errx(1, "42sh: fd_save: Failed dup file descriptor");
  if (fcntl(copy, F_SETFD, FD_CLOEXEC) < 0)
    errx(1, "42sh: fd_save: Failed fcntl file descriptor");
  if (close(fd) < 0)
    errx(1, "42sh: fd_save: Failed closing file descriptor");
  return copy;
}


static void fd_load(int copy, int origin)
{
  if (dup2(copy, origin) < 0)
    errx(1, "42sh: fd_load: Failed dup file descriptor");

  if (close(copy) < 0)
    errx(1, "42sh: fd_load: Failed closing file descriptor");
}


static int redir_great(s_env *env, s_aredirection *aredir, s_ast *cmd)
{
  int stdout_copy = fd_save(STDOUT_FILENO);

  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY, 0664);
  if (fd_file < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = 0;
  if (aredir->action)
    res = redirection_exec(env, aredir->action, cmd);
  else
    res = ast_exec(env, cmd);

  if (close(fd_file) < 0)
    errx(1, "42sh: redir_great: Failed closing %s", aredir->right->str);

  fd_load(stdout_copy, STDOUT_FILENO);

  return res;
}


static int redir_dgreat(s_env *env, s_aredirection *aredir, s_ast *cmd)
{
  int stdout_copy = fd_save(STDOUT_FILENO);

  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY | O_APPEND, 0664);
  if (fd_file < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = 0;
  if (aredir->action)
    res = redirection_exec(env, aredir->action, cmd);
  else
    res = ast_exec(env, cmd);

  if (close(fd_file) < 0)
    errx(1, "42sh: redir_great: Failed closing %s", aredir->right->str);

  fd_load(stdout_copy, STDOUT_FILENO);

  return res;
}


int redirection_exec(s_env *env, s_ast *ast, s_ast *cmd)
{
  s_aredirection *aredirection = &ast->data.ast_redirection;
  if (aredirection->type == REDIR_GREAT)
    return redir_great(env, aredirection, cmd);
  if (aredirection->type == REDIR_DGREAT)
    return redir_dgreat(env, aredirection, cmd);
  return 0;
}
