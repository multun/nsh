#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>

static int file_io_reader(s_cstream *cs)
{
    return getc(cs->data);
}

s_cstream *cstream_from_file(FILE *stream, const char *source, bool exit_close)
{
    s_cstream *cs = cstream_create_base();
    cs->exit_close = exit_close;
    cs->line_info = LINEINFO(source, NULL);
    cs->interactive = false;
    cs->backend = &g_io_file_backend;
    cs->data = stream;
    return cs;
}

static void file_io_dest(s_cstream *cs)
{
    if (cs->exit_close)
        fclose(cs->data);
}

s_io_backend g_io_file_backend = {
    .reader = file_io_reader,
    .dest = file_io_dest,
};
