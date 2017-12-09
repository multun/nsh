#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "utils/hash_table.h"


void cmd_print(FILE *f, s_ast *node)
{ void *id = node;
  s_wordlist *wl = node->data.ast_cmd.wordlist;
  fprintf(f, "\"%p\" [label=\"CMD\\n%s", id, wl->str);
  wl = wl->next;
  while (wl)
  {
    fprintf(f, " %s", wl->str);
    wl = wl->next;
  }
  fprintf(f, "\"];\n");
}


static void argv_free(char *argv[])
{
  for (size_t i = 0; argv[i]; i++)
    free(argv[i]);
  free(argv);
}


int cmd_exec_argv(s_env *env, s_errcont *cont, char *argv[])
{
  s_ast *func = htable_access(env->functions, argv[0]);
  if (func)
  {
    // TODO manage args
    return ast_exec(env, func, cont);
  }

  int status;
  pid_t pid = fork();

  if (pid < 0)
    clean_err(cont, errno, "cmd_exec: error while forking");

  else if (pid == 0)
  {
    execvp(*argv, argv);
    clean_err(cont, errno, "couldn't exec \"%s\"", *argv);
  }

  else
  {
    waitpid(pid, &status, 0);
    int res = WEXITSTATUS(status);
    return res;
  }
}

int cmd_exec(s_env *env, s_ast *node, s_errcont *cont)
{
  s_wordlist *wl = node->data.ast_cmd.wordlist;
  char **argv = wordlist_to_argv(wl, env);
  s_keeper keeper = KEEPER(cont->keeper);

  int res = 0;
  if (setjmp(keeper.env))
  {
    argv_free(argv);
    shraise(cont, NULL);
  }
  else
    res = cmd_exec_argv(env, &ERRCONT(cont->errman, &keeper), argv);
  argv_free(argv);
  return res;
}


void cmd_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_cmd.wordlist, true);
  free(ast);
}
