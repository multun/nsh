#include <stdio.h>

#include "ast/ast.h"


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
