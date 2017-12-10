#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "shexec/args.h"
#include "shexec/builtins.h"
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


int cmd_exec_argv(s_env *env, s_errcont *cont)
{
  struct pair *p = htable_access(env->functions, env->argv[0]);
  if (p)
    return ast_exec(env, p->value, cont);

  f_builtin builtin = builtin_search(env->argv[0]);
  if (builtin)
    return builtin(argv_count(env->argv), env->argv);

  int status;
  pid_t pid = fork();

  if (pid < 0)
    clean_err(cont, errno, "cmd_exec: error while forking");
  else if (pid == 0)
  {
    execvp(env->argv[0], env->argv);
    clean_err(cont, errno, "couldn't exec \"%s\"", env->argv[0]);
  }
  else
  {
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

int cmd_exec(s_env *env, s_ast *node, s_errcont *cont)
{
  s_wordlist *wl = node->data.ast_cmd.wordlist;
  s_env nenv = *env;
  nenv.argv = wordlist_to_argv(wl, env);
  s_keeper keeper = KEEPER(cont->keeper);

  int res = 0;
  if (setjmp(keeper.env))
  {
    argv_free(nenv.argv);
    shraise(cont, NULL);
  }
  else
    res = cmd_exec_argv(&nenv, &ERRCONT(cont->errman, &keeper));
  argv_free(nenv.argv);
  return res;
}


void cmd_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_cmd.wordlist, true);
  free(ast);
}
