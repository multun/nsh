#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>

static int string_io_reader(s_cstream *cs)
{
    char *str = cs->data;
    if (!*str)
        return EOF;

    int res = *(str++);
    cs->data = str;
    return res;
}

void cstream_string_init(struct cstream *cs, char *string, const char *source) {
    cs->line_info = LINEINFO(source);
    cs->backend = &g_io_string_backend;
    cs->interactive = false;
    cs->data = string;
}

// TODO: deprecate
s_cstream *cstream_from_string(char *string, const char *source)
{
    s_cstream *cs = cstream_create_base();
    cstream_string_init(cs, string, source);
    return cs;
}

s_io_backend g_io_string_backend = {
    .reader = string_io_reader,
    .dest = NULL,
};
