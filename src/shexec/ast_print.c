#include <stdio.h>

#include "ast/ast.h"

static void print_rec(FILE *f, s_ast *ast);


static void cmd_print(FILE *f, s_ast *node)
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


static void if_print(FILE *f, s_ast *node)
{
  s_aif *aif = &node->data.ast_if;
  void *id = node;
  fprintf(f, "\"%p\" [label=\"IF\"];\n", id);
  print_rec(f, aif->condition);
  void *id_cond = aif->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"CONDITION\"]\n", id, id_cond);
  print_rec(f, aif->success);
  void *id_succ = aif->success;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"THEN\"]\n", id, id_succ);
  if (aif->failure)
  {
    print_rec(f, aif->failure);
    void *id_fail = aif->failure;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"ELSE\"]\n", id, id_fail);
  }
}


static void print_rec(FILE *f, s_ast *ast)
{
  if (ast->type == SHNODE_CMD)
    cmd_print(f, ast);
  else if (ast->type == SHNODE_IF)
    if_print(f, ast);
  else if (ast->type == SHNODE_FOR)
    return;
  else if (ast->type == SHNODE_WHILE)
    return;
  else if (ast->type == SHNODE_UNTIL)
    return;
  else if (ast->type == SHNODE_REDIRECTION)
    return;
  else if (ast->type == SHNODE_PIPE)
    return;
  else if (ast->type == SHNODE_CASE)
    return;
  else if (ast->type == SHNODE_BOOL_OP)
    return;
  else if (ast->type == SHNODE_LIST)
    return;
  else if (ast->type == SHNODE_ASSIGNEMENT)
    return;
  else if (ast->type == SHNODE_FUNCTION)
    return;
}


void ast_print(FILE *f, s_ast *ast)
{
  fprintf(f, "digraph G {\n");
  print_rec(f, ast);
  fprintf(f, "}\n");
}
