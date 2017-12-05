#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "shexec/environment.h"


void cmd_print(FILE *f, s_ast *node)
{
  void *id = node;
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


int cmd_exec(s_env *env, s_ast *node)
{
  s_wordlist *wl = node->data.ast_cmd.wordlist;
  char **argv = wordlist_to_argv(wl, env);

  int status;
  pid_t pid = fork();

  if (pid < 0)
    err(1, "cmd_exec: error while forking");

  else if (pid == 0)
    err(execvp(*argv, argv), "exec");

  else
  {
    waitpid(pid, &status, 0);
    int res = WEXITSTATUS(status);
    free(argv);
    return res;
  }
}


void cmd_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_cmd.wordlist, true);
  free(ast);
}
