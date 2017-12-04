#include "ast/ast.h"

#include <stdio.h>
#include <string.h>


void case_print(FILE *f, s_ast *ast)
{
  s_acase *acase = &ast->data.ast_case;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"CASE\"];\n", id);
  s_acase_node *node = acase->nodes;

  while (node)
  {
    ast_print_rec(f, node->action);
    void *id_next = node->action;
    s_wordlist *pattern = node->pattern;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"%s", id, id_next, pattern->str);
    pattern = pattern->next;
    while (pattern)
    {
      fprintf(f, "\n%s", pattern->str);
      pattern = pattern->next;
    }
    fprintf(f, "\"];\n");
    node = node->next;
  }
}


int case_exec(s_env *env, s_ast *ast)
{
  s_acase *acase = &ast->data.ast_case;
  s_acase_node *node = acase->nodes;

  while (node)
  {
    s_wordlist *pattern = node->pattern;
    while (pattern)
    {
      if (!strcmp(pattern->str, pattern->str))
        return ast_exec(env, node->action);
      pattern = pattern->next;
    }
    node = node->next;
  }
  return 0;
}
