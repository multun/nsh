#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>




bool cstream_eof(s_cstream *cs)
{
  return !(cs->has_buf && cs->buf != EOF) && cs->eof;
}


static inline int cstream_get(s_cstream *cs)
{
  int res = cs->backend->reader(cs);
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


void cstream_free(s_cstream *cs)
{
  cs->backend->dest(cs);
  free(cs);
}
