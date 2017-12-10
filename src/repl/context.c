#include "ast/ast_list.h"
#include "cli/cmdopts.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/environment.h"


int context_init(s_context *cont, int argc, char *argv[])
{
  cont->ast_list = NULL;
  // these are initialized here in case managed_stream_init fails
  cont->env = NULL;
  cont->history = NULL;

  int res = managed_stream_init(cont, &cont->ms, argc, argv);
  if (res)
    return res;

  cont->env = environment_create(argv + (g_cmdopts.src == SHSRC_COMMAND));
  history_init(cont);
  return 0;
}


void context_destroy(s_context *cont)
{
  ast_list_free(cont->ast_list);
  environment_free(cont->env);
  history_destroy(cont);
  managed_stream_destroy(&cont->ms);
}
