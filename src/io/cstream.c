#include "io/cstream.h"

#include <stdlib.h>


void cstream_init_file(s_cstream *cs, FILE *stream, char *source)
{
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;
  cs->eof = false;

  cs->type = CSTREAM_BACKEND_FILE;
  cs->data.stream = stream;
}


void cstream_init_string(s_cstream *cs, char *string, char *source)
{
  cs->line_info = LINEINFO(source);
  cs->has_buf = false;
  cs->eof = false;

  cs->type = CSTREAM_BACKEND_STRING;
  cs->data.string = string;
}


bool cstream_eof(s_cstream *cs)
{
  return !cs->has_buf && cs->eof;
}


static inline int cstream_get(s_cstream *cs)
{
  int res;
  if (cs->type == CSTREAM_BACKEND_FILE)
    res = getc(cs->data.stream);
  else if (cs->type == CSTREAM_BACKEND_STRING)
    res = !*cs->data.string ? EOF : *(cs->data.string++);
  else
    abort();

  if (res == EOF)
    cs->eof = true;

  return res;
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
