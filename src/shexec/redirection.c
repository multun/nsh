#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
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
    redir = ">&";
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


static int exec_redir_base(s_env *env, s_ast *cmd,
                           s_errcont *cont, s_aredirection *aredir)
{
  if (aredir->action)
    return redirection_exec(env, aredir->action, cmd, cont);
  else
    return ast_exec(env, cmd, cont);
}


static int redir_great(s_env *env, s_aredirection *aredir,
                       s_ast *cmd, s_errcont *cont)
{
  int stdout_copy = fd_save(STDOUT_FILENO);

  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY, 0664);
  if (fd_file < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(fd_file) < 0)
    errx(1, "42sh: redir_great: Failed closing %s", aredir->right->str);

  fd_load(stdout_copy, STDOUT_FILENO);

  return res;
}


static int redir_dgreat(s_env *env, s_aredirection *aredir,
                        s_ast *cmd, s_errcont *cont)
{
  int stdout_copy = fd_save(STDOUT_FILENO);

  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY | O_APPEND, 0664);
  if (fd_file < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(fd_file) < 0)
    errx(1, "42sh: redir_dgreat: Failed closing %s", aredir->right->str);

  fd_load(stdout_copy, STDOUT_FILENO);

  return res;
}


static int redir_less(s_env *env, s_aredirection *aredir,
                      s_ast *cmd, s_errcont *cont)
{
  int copy = fd_save(STDIN_FILENO);

  int fd_file = open(aredir->right->str, O_RDONLY, 0664);
  if (fd_file < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(fd_file) < 0)
    errx(1, "42sh: redir_less: Failed closing %s", aredir->right->str);

  fd_load(copy, STDIN_FILENO);

  return res;
}


static int redir_lessand(s_env *env, s_aredirection *aredir,
                         s_ast *cmd, s_errcont *cont)
{
  if (aredir->left == -1)
    aredir->left = 0;
  if (!strcmp("-", aredir->right->str))
    if (close(aredir->left) < 0)
      errx(1, "42sh: redir_lessand: Failed closing %d", aredir->left);
  int digit = atoi(aredir->right->str);
  if (dup2(digit, aredir->left) < 0)
  {
    warnx("42sh: %s: Bad file descriptor", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(aredir->left) < 0)
    errx(1, "42sh: redir_lessand: Failed closing %d", aredir->left);
  return res;
}


static int redir_greatand(s_env *env, s_aredirection *aredir,
                          s_ast *cmd, s_errcont *cont)
{
  if (aredir->left == -1)
    aredir->left = 1;
  if (!strcmp("-", aredir->right->str))
    if (close(aredir->left) < 0)
      errx(1, "42sh: redir_greatand: Failed closing %d", aredir->left);
  int digit = atoi(aredir->right->str);
  if (dup2(digit, aredir->left) < 0)
  {
    warnx("42sh: %s: Bad file descriptor", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(aredir->left) < 0)
    errx(1, "42sh: redir_lessand: Failed closing %d", aredir->left);
  return res;
}


static int redir_lessgreat(s_env *env, s_aredirection *aredir,
                           s_ast *cmd, s_errcont *cont)
{
  if (aredir->left == -1)
    aredir->left = 0;
  int fd = open(aredir->right->str, O_CREAT | O_RDWR, 0664);
  if (fd < 0)
  {
    warnx("42sh: %s: Permission denied\n", aredir->right->str);
    return 1;
  }
  if (dup2(fd, aredir->left) < 0)
    errx(1, "42sh: redir_lessgreat: Failed dup file descriptor");
  if (close(fd) < 0)
    errx(1, "42sh: redir_lessgreat: Failed closing file descriptor");

  int res = exec_redir_base(env, cmd, cont, aredir);

  if (close(aredir->left) < 0)
    errx(1, "42sh: redir_lessgreat: Failed closing %d", aredir->left);
  return res;
}


int redirection_exec(s_env *env, s_ast *ast, s_ast *cmd, s_errcont *cont)
{
  s_aredirection *aredirection = &ast->data.ast_redirection;
  if (aredirection->type == REDIR_GREAT)
    return redir_great(env, aredirection, cmd, cont);
  if (aredirection->type == REDIR_DGREAT)
    return redir_dgreat(env, aredirection, cmd, cont);
  if (aredirection->type == REDIR_LESS)
    return redir_less(env, aredirection, cmd, cont);
  if (aredirection->type == REDIR_LESSAND)
    return redir_lessand(env, aredirection, cmd, cont);
  if (aredirection->type == REDIR_GREATAND)
    return redir_greatand(env, aredirection, cmd, cont);
  if (aredirection->type == REDIR_LESSGREAT)
    return redir_lessgreat(env, aredirection, cmd, cont);
  return 0;
}
