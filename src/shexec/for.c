#include <stdio.h>

#include "ast/ast.h"


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
