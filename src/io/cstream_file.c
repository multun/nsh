#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>


int file_io_reader(s_cstream *cs)
{
  return getc(cs->data);
}


s_cstream *cstream_from_file(FILE *stream, char *source)
{
  s_cstream *cs = cstream_create_base();
  cs->line_info = LINEINFO(source);
  cs->backend = &g_io_file_backend;
  cs->data = stream;
  return cs;
}


s_io_backend g_io_file_backend =
{
  .reader = file_io_reader,
  .dest = NULL,
};
