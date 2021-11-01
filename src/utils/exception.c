#include <nsh_utils/alloc.h>
#include <nsh_utils/exception.h>
#include <nsh_utils/progname.h>

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

void vshwarn(const struct lineinfo *li, const char *format, va_list ap)
{
    fprintf(stderr, "%s: ", program_name);
    lineinfo_print(li, stderr);
    fprintf(stderr, ": ");
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
}

void shwarn(const struct lineinfo *li, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vshwarn(li, format, ap);

    va_end(ap);
}

void __noreturn vsherror(const struct lineinfo *li, struct exception_catcher *catcher,
                         const struct exception_type *exception_type, const char *format,
                         va_list ap)
{
    assert(catcher);
    catcher->context->class = exception_type;
    vshwarn(li, format, ap);
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
