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
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);
  print_rec(f, aif->success);
  void *id_succ = aif->success;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"THEN\"];\n", id, id_succ);
  if (aif->failure)
  {
    print_rec(f, aif->failure);
    void *id_fail = aif->failure;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"ELSE\"];\n", id, id_fail);
  }
}


static void for_print(FILE *f, s_ast *ast)
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
  print_rec(f, afor->actions);
  void *id_do = afor->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}


static void while_print(FILE *f, s_ast *ast)
{
  s_awhile *awhile = &ast->data.ast_while;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"WHILE\"];\n", id);

  print_rec(f, awhile->condition);
  void *id_cond = awhile->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);

  print_rec(f, awhile->actions);
  void *id_do = awhile->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}


static void until_print(FILE *f, s_ast *ast)
{
  s_auntil *auntil = &ast->data.ast_until;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"UNTIL\"];\n", id);

  print_rec(f, auntil->condition);
  void *id_cond = auntil->condition;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n", id, id_cond);

  print_rec(f, auntil->actions);
  void *id_do = auntil->actions;
  fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}


static void bool_op_print(FILE *f, s_ast *ast)
{
  s_abool_op *abool = &ast->data.ast_bool_op;
  void *id = ast;
  if (abool->type == BOOL_OR)
    fprintf(f, "\"%p\" [label=\"OR\"];\n", id);
  else if (abool->type == BOOL_AND)
    fprintf(f, "\"%p\" [label=\"AND\"];\n", id);
  else
    fprintf(f, "\"%p\" [label=\"NOT\"];\n", id);

  print_rec(f, abool->left);
  void *id_left = abool->left;
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

  if (abool->type != BOOL_NOT)
  {
    print_rec(f, abool->right);
    void *id_right = abool->right;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
  }
}


static void list_print(FILE *f, s_ast *ast)
{
  s_alist *alist = &ast->data.ast_list;

  if (alist)
  {
    print_rec(f, alist->action);
    if (alist->next)
    {
      void *id = alist->action;
      void *id_next = alist->next->action;
      fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
    }
    alist = alist->next;
  }
}


static void print_rec(FILE *f, s_ast *ast)
{
  if (ast->type == SHNODE_CMD)
    cmd_print(f, ast);
  else if (ast->type == SHNODE_IF)
    if_print(f, ast);
  else if (ast->type == SHNODE_FOR)
    for_print(f, ast);
  else if (ast->type == SHNODE_WHILE)
    while_print(f, ast);
  else if (ast->type == SHNODE_UNTIL)
    until_print(f, ast);
  else if (ast->type == SHNODE_REDIRECTION)
    return;
  else if (ast->type == SHNODE_PIPE)
    return;
  else if (ast->type == SHNODE_CASE)
    return;
  else if (ast->type == SHNODE_BOOL_OP)
    bool_op_print(f, ast);
  else if (ast->type == SHNODE_LIST)
    list_print(f, ast);
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
