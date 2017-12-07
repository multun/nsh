#include "utils/alloc.h"
#include "utils/error.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>


void shraise(s_errcont *cont, const s_ex_class *class)
{
  if (!class)
    class = cont->errman->class;
  if (!class)
    abort();

  longjmp(cont->keeper->env, 1);
}


static const struct ex_class test_class;

int sherror(const s_lineinfo *li, s_errcont *cont, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  assert(cont);
  cont->errman->class = &test_class;

  fprintf(stderr, "%s:%zu:%zu: ", li->source, li->line,
          li->column);

  vfprintf(stderr, format, ap);
  fputc('\n', stderr);


  shraise(cont, NULL);

  va_end(ap);
  return true;
}
