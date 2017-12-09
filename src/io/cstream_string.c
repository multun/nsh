#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>


int string_io_reader(s_cstream *cs)
{
  char *str = cs->data;
  if (!*str)
    return EOF;

  int res = *(str++);
  cs->data = str;
  return res;
}


s_cstream *cstream_from_string(char *string, char *source)
{
  s_cstream *cs = cstream_create_base();
  cs->line_info = LINEINFO(source);
  cs->backend = &g_io_string_backend;
  cs->data = string;
  return cs;
}


s_io_backend g_io_string_backend =
{
  .reader = string_io_reader,
  .dest = NULL,
};
