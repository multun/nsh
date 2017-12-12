#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"


void for_print(FILE *f, s_ast *ast)
{
  s_afor *afor = &ast->data.ast_for;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"FOR %s in", id, afor->var->str);
  s_wordlist *wl = afor->collection;
  while (wl)
  {
    fprintf(f, " %s", wl->str);
    wl = wl->next;
  }
  fprintf(f, "\"];\n");
  ast_print_rec(f, afor->actions);
  void *id_do = afor->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}



static void for_exception_handler(volatile bool *local_continue,
                                  s_errcont *cont,
                                  s_env *env)
{
  // the break builtin ensures no impossible break is emitted
  if (cont->errman->class != &g_lbreak || --env->break_count)
  {
    env->depth--;
    shraise(cont, NULL);
  }
  *local_continue = env->break_continue;
}

int for_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_afor *afor = &ast->data.ast_for;

  volatile int ret = 0;
  volatile bool local_continue = true;
  s_wordlist *volatile wl = afor->collection;

  env->depth++;
  s_keeper keeper = KEEPER(cont->keeper);
  s_errcont ncont = ERRCONT(cont->errman, &keeper);
  if (setjmp(keeper.env))
    for_exception_handler(&local_continue, cont, env);

  if (local_continue)
    for (; wl; wl = wl->next)
    {
      assign_var(env, strdup(afor->var->str), expand(wl->str, env));
      ret = ast_exec(env, afor->actions, &ncont);
    }

  env->depth--;
  return ret;
}


void for_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_for.var, true);
  wordlist_free(ast->data.ast_for.collection, true);
  ast_free(ast->data.ast_for.actions);
  free(ast);
}
