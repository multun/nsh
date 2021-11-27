#include <nsh_utils/alloc.h>
#include <nsh_utils/exception.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>


void __noreturn shraise(struct exception_catcher *catcher,
                        const struct exception_type *class)
{
    if (class)
        catcher->context->class = class;
    longjmp(catcher->env, 1);
}


void __noreturn shreraise(struct exception_catcher *catcher)
{
    shraise(catcher, NULL);
}

void __noreturn vsherror(const struct lineinfo *li, struct exception_catcher *catcher,
                         const struct exception_type *exception_type, const char *format,
                         va_list ap)
{
    assert(catcher);
    catcher->context->class = exception_type;
    lineinfo_vwarn(li, format, ap);
    shraise(catcher, NULL);
}

void __noreturn sherror(const struct lineinfo *li, struct exception_catcher *catcher,
                        const struct exception_type *exception_type, const char *format,
                        ...)
{
    va_list ap;
    va_start(ap, format);

    vsherror(li, catcher, exception_type, format, ap);

    va_end(ap);
}
