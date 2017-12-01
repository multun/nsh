#include "io/cstream.h"

#include <stdlib.h>


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
  int res;

  if (cs->has_buf)
  {
    cs->has_buf = false;
    res = cs->buf;
  }
  else
    res = cstream_get(cs);

  if (res == '\n')
  {
    cs->line_info.line++;
    cs->line_info.column = 1;
  }
  else
    cs->line_info.column++;

  return res;
}
