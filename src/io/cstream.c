#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>



s_cstream *cstream_create_base(void)
{
  s_cstream *cs = xmalloc(sizeof(*cs));
  cs->linebuf =
  (s_evect)
  {
    0
  };
  cs->has_buf = false;
  cs->eof = false;
  cs->back_pos = 0;
  cs->backend = NULL;
  cs->data = NULL;
  return cs;
}


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

  if (EVECT_INITIALIZED(&cs->linebuf) && res != EOF)
    evect_push(&cs->linebuf, res);
  return res;
}


void cstream_free(s_cstream *cs)
{
  if (!cs)
    return;

  if (EVECT_INITIALIZED(&cs->linebuf))
    evect_destroy(&cs->linebuf);
  if (cs->backend->dest)
    cs->backend->dest(cs);

  free(cs);
}
