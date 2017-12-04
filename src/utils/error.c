#include "utils/alloc.h"
#include "utils/error.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


int sherror(const s_lineinfo *li, s_errman *errman, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  assert(errman);
  errman->panic = true;

  fprintf(stderr, "%s:%zu:%zu: ", li->source, li->line,
          li->column);

  vfprintf(stderr, format, ap);
  fputc('\n', stderr);

  va_end(ap);
  return true;
}
