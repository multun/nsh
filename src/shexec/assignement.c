#include <stdio.h>

#include "ast/ast.h"


void assignement_print(FILE *f, s_ast *ast)
{
  s_aassignement *aassignement = &ast->data.ast_assignement;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"%s = %s\"];\n", id, aassignement->name->str,
          aassignement->value->str);
  void *id_next = aassignement->action;
  ast_print_rec(f, aassignement->action);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}
