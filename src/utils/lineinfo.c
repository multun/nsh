#include <stdio.h>

#include <nsh_utils/lineinfo.h>

void lineinfo_print(const struct lineinfo *li, FILE *stream)
{
    if (li->parent) {
        lineinfo_print(li->parent, stream);
        fprintf(stream, " => ");
    }

    fprintf(stream, "%s:%zu:%zu", li->source, li->line, li->column);
}
