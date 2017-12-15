#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ast/ast.h"
#include "utils/macros.h"
#include "shexec/redirection_utils.h"

struct redir_params
{
  s_env *env;
  s_aredirection *aredir;
  s_ast *cmd;
  s_errcont *cont;
};

static int redirection_local_exec(struct redir_params *params);


static int exec_redir_base(struct redir_params *params)
{
  if (params->aredir->action)
  {
    params->aredir = &params->aredir->action->data.ast_redirection;
    return redirection_local_exec(params);
  }
  return ast_exec(params->env, params->cmd, params->cont);
}


static int redir_great(struct redir_params *params)
{
  int stdout_copy = fd_move_away(STDOUT_FILENO);

  s_aredirection *aredir = params->aredir;
  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY | O_TRUNC, 0664);
  if (fd_file < 0)
  {
    warnx("%s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(params);

  if (close(fd_file) < 0)
    errx(1, "redir_great: Failed closing %s", aredir->right->str);

  fd_move(stdout_copy, STDOUT_FILENO);

  return res;
}


static int redir_dgreat(struct redir_params *params)
{
  s_aredirection *aredir = params->aredir;
  int stdout_copy = fd_move_away(STDOUT_FILENO);

  int fd_file = open(aredir->right->str, O_CREAT | O_WRONLY | O_APPEND, 0664);
  if (fd_file < 0)
  {
    warnx("%s: Permission denied\n", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(params);

  if (close(fd_file) < 0)
    errx(1, "redir_dgreat: Failed closing %s", aredir->right->str);

  fd_move(stdout_copy, STDOUT_FILENO);

  return res;
}


static int redir_less(struct redir_params *params)
{
  s_aredirection *aredir = params->aredir;
  int copy = fd_move_away(STDIN_FILENO);

  int fd_file = open(aredir->right->str, O_RDONLY, 0664);
  if (fd_file < 0)
  {
    warn("cannot open \"%s\"", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(params);

  if (close(fd_file) < 0)
    errx(1, "redir_less: Failed closing %s", aredir->right->str);

  fd_move(copy, STDIN_FILENO);

  return res;
}


static int redir_lessand(struct redir_params *params)
{
  s_aredirection *aredir = params->aredir;
  if (aredir->left == -1)
    aredir->left = 0;

  if (!strcmp("-", aredir->right->str))
    if (close(aredir->left) < 0)
      errx(1, "redir_lessand: Failed closing %d", aredir->left);

  int digit = atoi(aredir->right->str);
  if (dup2(digit, aredir->left) < 0)
  {
    warnx("%s: Bad file descriptor", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(params);

  if (close(aredir->left) < 0)
    errx(1, "redir_lessand: Failed closing %d", aredir->left);
  return res;
}


static int redir_greatand(struct redir_params *params)
{
  s_aredirection *aredir = params->aredir;
  if (aredir->left == -1)
    aredir->left = 1;
  if (!strcmp("-", aredir->right->str))
    if (close(aredir->left) < 0)
      errx(1, "redir_greatand: Failed closing %d", aredir->left);

  int save = fd_copy(aredir->left);
  int digit = atoi(aredir->right->str);
  if (dup2(digit, aredir->left) < 0)
  {
    warnx("%s: Bad file descriptor", aredir->right->str);
    return 1;
  }

  int res = exec_redir_base(params);

  if (close(aredir->left) < 0)
    errx(1, "redir_lessand: Failed closing %d", aredir->left);
  dup2(save, aredir->left);
  close(save);
  return res;
}


static int redir_lessgreat(struct redir_params *params)
{
  s_aredirection *aredir = params->aredir;
  if (aredir->left == -1)
    aredir->left = 0;
  int fd = open(aredir->right->str, O_CREAT | O_RDWR | O_TRUNC, 0664);
  if (fd < 0)
  {
    warnx("%s: Permission denied\n", aredir->right->str);
    return 1;
  }
  if (dup2(fd, aredir->left) < 0)
    errx(1, "redir_lessgreat: Failed dup file descriptor");
  if (close(fd) < 0)
    errx(1, "redir_lessgreat: Failed closing file descriptor");

  int res = exec_redir_base(params);

  if (close(aredir->left) < 0)
    errx(1, "redir_lessgreat: Failed closing %d", aredir->left);
  return res;
}



static const struct redir_meta
{
  const char *repr;
  int (*func)(struct redir_params *params);
} g_redir_list[] =
{
#define REDIRECIONS_LIST(EName, Repr, Func) { Repr, Func },
  REDIRECTIONS_APPLY(REDIRECIONS_LIST)
};


static int redirection_local_exec(struct redir_params *params)
{
  if (params->aredir->type >= ARR_SIZE(g_redir_list))
    abort();

  const struct redir_meta *rmeta = &g_redir_list[params->aredir->type];
  if (rmeta->func)
    return rmeta->func(params);

  warnx("ignoring unimplemented redirection \"%s\"", rmeta->repr);
  return exec_redir_base(params);
}


void redirection_print(FILE *f, s_ast *ast)
{
  s_aredirection *aredirection = &ast->data.ast_redirection;
  void *id = ast;

  if (aredirection->type >= ARR_SIZE(g_redir_list))
    abort();

  const char *redir = g_redir_list[aredirection->type].repr;

  fprintf(f, "\"%p\" [label=\"%d %s %s\"];\n", id, aredirection->left,
          redir, aredirection->right->str);

  if (aredirection->action)
  {
    ast_print_rec(f, aredirection->action);
    void *id_next = aredirection->action;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
  }
}


int redirection_exec(s_env *env, s_ast *ast, s_ast *cmd, s_errcont *cont)
{
  struct redir_params params =
  {
    env, &ast->data.ast_redirection, cmd, cont
  };
  return redirection_local_exec(&params);
}
