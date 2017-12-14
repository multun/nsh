#include <stdio.h>
#include <string.h>

#include "ast/ast.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/alloc.h"


void assignment_print(FILE *f, s_ast *ast)
{
  s_aassignment *aassignment = &ast->data.ast_assignment;
  void *id = ast;
  fprintf(f, "\"%p\" [label=\"%s = %s\"];\n", id, aassignment->name->str,
          aassignment->value->str);
  void *id_next = aassignment->action;
  ast_print_rec(f, aassignment->action);
  fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}


void assign_var(s_env *env, char *name, char *value, bool export)
{
  struct pair *prev = htable_access(env->vars, name);
  if (prev)
  {
    s_var *var = prev->value;
    free(var->value);
    free(name);
    var->value = value;
    if (export)
      var->to_export = true;
    return;
  }
  s_var *nvar = xmalloc(sizeof(s_var));
  *nvar = VARIABLE(value);
  nvar->to_export = export;
  htable_add(env->vars, name, nvar);
}


int assignment_exec(s_env *env, s_ast *ast, s_ast *cmd, s_errcont *cont)
{
  if (!ast)
    return ast_exec(env, cmd, cont);
  char *name = strdup(ast->data.ast_assignment.name->str);
  char *value = expand(ast->data.ast_assignment.value->str, env, cont);
  assign_var(env, name, value, cmd != NULL);
  return assignment_exec(env, ast->data.ast_assignment.action,
                           cmd, cont);
}


void assignment_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_assignment.name, true);
  wordlist_free(ast->data.ast_assignment.value, false);
  ast_free(ast->data.ast_assignment.action);
  free(ast);
}
