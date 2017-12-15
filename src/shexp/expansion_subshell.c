#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/error.h"
#include "utils/evect.h"

#include <err.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


static int subshell_child(s_env *env, char *str)
{
  s_context ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.env = env;
  ctx.cs = cstream_from_string(str, "<subshell>");
  repl(&ctx);
  int rc = ctx.env->code;
  ctx.env = NULL; // avoid double free
  context_destroy(&ctx);
  return rc;
}


void subshell_parent(int cfd, s_evect *res)
{
  FILE *creader = fdopen(cfd, "r");
  int cur_char;

  while ((cur_char = fgetc(creader)) != EOF)
    evect_push(res, cur_char);

  char c = 0;
  while (res->size > 1 && (c = res->data[res->size - 1]) == '\n')
    res->size--;

  fclose(creader);
}


static char *subshell_find_par(char *str)
{
  size_t count = 1;
  while (count && *str)
  {
    if (*str == '\\')
    {
      str += 2;
      continue;
    }
    if (*str == '(')
      count++;
    else if (*str == ')')
      count--;
    if (count)
      str++;
  }
  return str;
}

void expand_subshell(s_errcont *cont, char **str, s_env *env, s_evect *vec)
{
  char *buf = strndup(*str, subshell_find_par(*str) - *str);
  // TODO: subshell error handling
  int pfd[2];
  pipe(pfd);
  int cpid = fork();
  if (!cpid)
  {
    close(pfd[0]);
    dup2(pfd[1], 1);
    int res = subshell_child(env, buf);
    free(buf);
    evect_destroy(vec);
    close(pfd[1]);
    clean_exit(cont, res);
  }

  free(buf);
  close(pfd[1]);
  subshell_parent(pfd[0], vec);
  close(pfd[0]);
  int status;
  waitpid(cpid, &status, 0);
  *str = subshell_find_par(*str) + 1;
}


void expand_arth(char **str, s_env *env, s_evect *vec)
{
  if (!*str && !env && !vec)
    warnx("expand_arth: not implemented yet");
}
