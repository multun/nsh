#include "utils/alloc.h"
#include "utils/error.h"
#include "utils/progname.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

void ATTR(noreturn) shraise(struct errcont *cont, const struct ex_class *class)
{
    if (class)
        cont->errman->class = class;
    longjmp(cont->env, 1);
}

void vshwarn(const struct lineinfo *li, const char *format, va_list ap)
{

    fprintf(stderr, "%s: ", program_name);
    lineinfo_print(li, stderr);
    fprintf(stderr, ": ");
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
}

void ATTR(noreturn) vsherror(const struct lineinfo *li, struct errcont *cont,
                             const struct ex_class *ex_class, const char *format, va_list ap)
{
    assert(cont);
    cont->errman->class = ex_class;
    vshwarn(li, format, ap);
    shraise(cont, NULL);
}

void ATTR(noreturn) sherror(const struct lineinfo *li, struct errcont *cont,
                            const struct ex_class *ex_class, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vsherror(li, cont, ex_class, format, ap);

    va_end(ap);
}
