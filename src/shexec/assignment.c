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


void assign_var(s_env *env, char *name, char *value)
{
  struct pair *prev = htable_access(env->vars, name);
  if (prev)
  {
    struct pair p = *prev;
    htable_remove(env->vars, name);
    s_var *var = p.value;
    free(p.key);
    free(var->value);
    free(var);
  }

  s_var *nvar = xmalloc(sizeof(s_var));
  *nvar = VARIABLE(value);
  htable_add(env->vars, name, nvar);
}


int assignment_exec(s_env *env, s_ast *ast, s_errcont *cont)
{
  char *name = strdup(ast->data.ast_assignment.name->str);
  char *value = expand(ast->data.ast_assignment.value->str, env);
  assign_var(env, name, value);
  if (cont)
    return 0;
  return 0;
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
