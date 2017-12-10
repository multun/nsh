#include "ast/ast_list.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/environment.h"

void context_init(s_context *cont, char *argv[])
{
  cont->ast_list = NULL;
  cont->env = environment_create(argv);
}


void context_destroy(s_context *cont)
{
  ast_list_free(cont->ast_list);
  environment_free(cont->env);
  history_destroy(cont);
}
