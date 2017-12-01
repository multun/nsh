#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>


s_cstream *cstream_from_file(FILE *stream, char *source)
{
  s_cstream *cs = xmalloc(sizeof(*cs));
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;
  cs->eof = false;

  cs->type = CSTREAM_BACKEND_FILE;
  cs->data.stream = stream;
  return cs;
}


s_cstream *cstream_from_string(char *string, char *source)
{
  s_cstream *cs = xmalloc(sizeof(*cs));
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;
  cs->eof = false;

  cs->type = CSTREAM_BACKEND_STRING;
  cs->data.string = string;
  return cs;
}


void cstream_free(s_cstream *cs)
{
  free(cs);
}
