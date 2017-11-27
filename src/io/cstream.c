#include "io/cstream.h"

#include <stdlib.h>


void cstream_init_file(s_cstream *cs, FILE *stream, char *source)
{
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;

  cs->type = CSTREAM_BACKEND_FILE;
  cs->data.stream = stream;
}


void cstream_init_string(s_cstream *cs, char *string, char *source)
{
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;

  cs->type = CSTREAM_BACKEND_STRING;
  cs->data.string = string;
}


static inline int cstream_get(s_cstream *cs)
{
  if (cs->type == CSTREAM_BACKEND_FILE)
    return getc(cs->data.stream);
  if (cs->type == CSTREAM_BACKEND_STRING)
    return !*cs->data.string ? EOF : *(cs->data.string++);
  abort();
}


int cstream_peek(s_cstream *cs)
{
  if (!cs->has_buf)
  {
    cs->has_buf = true;
    cs->buf = cstream_get(cs);
  }
  return cs->buf;
}


int cstream_pop(s_cstream *cs)
{
  if (cs->has_buf)
  {
    cs->has_buf = false;
    return cs->buf;
  }

  return cstream_get(cs);
}
