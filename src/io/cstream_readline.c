#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>

#include <stdio.h>
// readline's header requires including stdio beforehand
#include <readline/history.h>
#include <readline/readline.h>


int readline_io_reader(s_cstream *cs)
{
  char *str = cs->data;

  int res;
  if (str && !str[cs->back_pos])
  {
    free(str);
    cs->data = NULL;
    return '\n';
  }

  if (!str)
  {
    str = cs->data = readline("42sh>");
    cs->back_pos = 0;
  }

  if (!str || !(res = str[cs->back_pos]))
    return EOF;

  cs->back_pos++;
  return res;
}


s_cstream *cstream_readline(void)
{
  s_cstream *cs = xmalloc(sizeof(*cs));
  cs->line_info = LINEINFO("<tty>");
  cs->has_buf = false;
  cs->eof = false;

  cs->back_pos = 0;
  cs->backend = &g_io_readline_backend;
  cs->data = NULL;
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
