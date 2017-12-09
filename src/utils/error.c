#include "utils/alloc.h"
#include "utils/error.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>


void ATTR(noreturn) shraise(s_errcont *cont, const s_ex_class *class)
{
  cont->errman->class = class;
  longjmp(cont->keeper->env, 1);
}


void ATTR(noreturn) sherror(const s_lineinfo *li, s_errcont *cont,
                           const s_ex_class *ex_class, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  assert(cont);
  cont->errman->class = ex_class;

  fprintf(stderr, "%s:%zu:%zu: ", li->source, li->line,
          li->column);

  vfprintf(stderr, format, ap);
  fputc('\n', stderr);


  shraise(cont, NULL);

  va_end(ap);
}
