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


static void var_free_from_pair(struct pair p)
{
  s_var *var = p.value;
  free(p.key);
  free(var->value);
  free(var);
}


void assign_var(s_env *env, char *name, char *value, bool rm_var)
{
  struct pair *prev = htable_access(env->vars, name);
  if (prev)
  {
    struct pair p = *prev;
    htable_remove(env->vars, name);
    if (rm_var)
      var_free_from_pair(p);
  }

  s_var *nvar = xmalloc(sizeof(s_var));
  *nvar = VARIABLE(value);
  htable_add(env->vars, name, nvar);
}


static void assignment_export(s_env *env, s_ast *ast)
{
  char *name = strdup(ast->data.ast_assignment.name->str);
  char *value = expand(ast->data.ast_assignment.value->str, env);
  assign_var(env, name, value, true);

  struct pair *p = htable_access(env->vars, name);
  s_var *var = p->value;
  var->to_export = true;
}


static struct pair assignment_local(s_env *env, s_ast *ast)
{
  char *name = strdup(ast->data.ast_assignment.name->str);
  char *value = expand(ast->data.ast_assignment.value->str, env);
  struct pair *prev = htable_access(env->vars, name);
  struct pair p;
  if (prev)
    p = *prev;
  else
    memset(&p, 0, sizeof(struct pair));

  assign_var(env, name, value, false);

  prev = htable_access(env->vars, name);
  s_var *var = prev->value;
  s_var *pvar = p.value;
  var->to_export = pvar->to_export;
  var->touched = false;
  return p;
}


static void unassigne_local(struct pair p, s_env *env)
{
  struct pair *prev = htable_access(env->vars, p.key);
  if (prev)
  {
    s_var *var = prev->value;
    if (var->touched)
    {
      var_free_from_pair(p);
      return;
    }
    var_free_from_pair(*prev);
    htable_remove(env->vars, p.key);
  }
  htable_add(env->vars, p.key, p.value);
}


int assignment_exec(s_env *env, s_ast *ast, s_ast *cmd, s_errcont *cont)
{
  if (!ast)
    return ast_exec(env, cmd, cont);
  if (!cmd)
  {
    assignment_export(env, ast);
    return assignment_exec(env, ast->data.ast_assignment.action,
                           cmd, cont);
  }
  struct pair p = assignment_local(env, ast);
  int res = assignment_exec(env, ast->data.ast_assignment.action,
                            cmd, cont);
  unassigne_local(p, env);
  return res;
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
