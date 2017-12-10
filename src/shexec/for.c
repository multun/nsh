#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
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


int for_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  s_afor *afor = &ast->data.ast_for;
  char *name = NULL;
  char *value = NULL;
  s_wordlist *wl = afor->collection;
  int ret = 0;
  while (wl)
  {
    name = xmalloc(strlen(afor->var->str) + 1);
    name = strcpy(name, afor->var->str);
    value = expand(wl->str, env);
    assign_var(env, name, value);
    ret = ast_exec(env, afor->actions, cont);
    wl = wl->next;
  }

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
