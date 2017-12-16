#include "ast/ast_list.h"
#include "cli/cmdopts.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/environment.h"
#include "utils/pathutils.h"

#include <pwd.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


static bool env_feed(s_env *env, const char *path, const char *name)
{
  s_context cont;
  memset(&cont, 0, sizeof(cont));
  FILE *file = fopen(path, "r");
  if (!file)
    return 0; // not being able to load an rc file is ok

  cont.env = env;
  cont.cs = cstream_from_file(file, name, true);

  bool should_exit = repl(&cont);

  cstream_free(cont.cs);
  return should_exit;
}


static bool context_load_rc(s_context *cont)
{
  const char global_rc[] = "/etc/42shrc";
  if (env_feed(cont->env, global_rc, global_rc))
    return true;

  char *rc_path = home_suffix("/.42shrc");
  bool should_exit = env_feed(cont->env, rc_path, "~/.42shrc");
  free(rc_path);
  return should_exit;
}


bool context_init(int *rc, s_context *cont, s_arg_context *arg_cont)
{
  // these are initialized here in case managed_stream_init fails
  cont->env = NULL;
  cont->history = NULL;

  if ((*rc = cstream_dispatch_init(cont, &cont->cs, arg_cont)))
    return true;

  cont->env = environment_create(arg_cont);

  if (cont->cs->interactive && !g_cmdopts.norc && context_load_rc(cont))
  {
    *rc = cont->env->code;
    return true;
  }


  history_init(cont);
  return 0;
}


void context_destroy(s_context *cont)
{
  environment_free(cont->env);
  history_destroy(cont);
  cstream_free(cont->cs);
}
