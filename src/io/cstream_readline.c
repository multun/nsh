#include "io/cstream.h"
#include "repl/repl.h"
#include "shexp/variable.h"
#include "utils/alloc.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
// readline's header requires including stdio beforehand
#include <readline/history.h>
#include <readline/readline.h>




static const char *cont_get_var(s_context *cont, const char *vname,
                                const char *def)
{
  struct pair *hpair = htable_access(cont->env->vars, vname);
  if (!hpair)
    return def;
  s_var *var = hpair->value;
  if (!var->value)
    return def;
  return var ? var->value : "";
}


static const char *get_ps1(s_context *context)
{
  return cont_get_var(context, "PS1", "42sh> ");
}


static const char *get_ps2(s_context *context)
{
  return cont_get_var(context, "PS2", "> ");
}


static const char *prompt_get(s_cstream *cs)
{
  const char *res = (cs->context->line_start ? get_ps1 : get_ps2)(cs->context);
  cs->context->line_start = false;
  return res;
}


static int readline_io_reader(s_cstream *cs)
{
  char *str = cs->data;

  if (!str)
  {
    const char *prompt = prompt_get(cs);
    str = cs->data = readline(prompt);
    cs->back_pos = 0;
  }

  int res;
  if (str && !str[cs->back_pos])
  {
    free(str);
    cs->data = NULL;
    return '\n';
  }

  if (!str || !(res = str[cs->back_pos]))
    return EOF;

  cs->back_pos++;
  return res;
}


s_cstream *cstream_readline(void)
{
  s_cstream *cs = cstream_create_base();
  cs->line_info = LINEINFO("<tty>");
  evect_init(&cs->linebuf, 100);
  cs->backend = &g_io_readline_backend;
  using_history();
  return cs;
}


static void readline_io_dest(s_cstream *cs)
{
  free(cs->data);
}


s_io_backend g_io_readline_backend =
{
  .reader = readline_io_reader,
  .dest = readline_io_dest,
};
