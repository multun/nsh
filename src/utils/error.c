#include "utils/alloc.h"
#include "utils/error.h"
#include "utils/progname.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

void ATTR(noreturn) shraise(s_errcont *cont, const s_ex_class *class)
{
    if (class)
        cont->errman->class = class;
    longjmp(cont->keeper->env, 1);
}

void ATTR(noreturn) vsherror(const s_lineinfo *li, s_errcont *cont,
                             const s_ex_class *ex_class, const char *format, va_list ap)
{
    assert(cont);
    cont->errman->class = ex_class;

    fprintf(stderr, "%s: ", program_name);
    lineinfo_print(li, stderr);
    fprintf(stderr, ": ");
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);

    shraise(cont, NULL);
}

void ATTR(noreturn) sherror(const s_lineinfo *li, s_errcont *cont,
                            const s_ex_class *ex_class, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vsherror(li, cont, ex_class, format, ap);

    va_end(ap);
}
