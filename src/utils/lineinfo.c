#include <stdio.h>
#include <stdarg.h>

#include <nsh_utils/lineinfo.h>
#include <nsh_utils/progname.h>

void lineinfo_print(const struct lineinfo *li, FILE *stream)
{
    if (li->parent) {
        lineinfo_print(li->parent, stream);
        fprintf(stream, " => ");
    }

    fprintf(stream, "%s:%zu:%zu", li->source, li->line, li->column);
}


void lineinfo_vwarn(const struct lineinfo *li, const char *format, va_list ap)
{
    fprintf(stderr, "%s: ", program_name);
    lineinfo_print(li, stderr);
    fprintf(stderr, ": ");
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
}


void lineinfo_warn(const struct lineinfo *li, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    lineinfo_vwarn(li, format, ap);

    va_end(ap);
}
