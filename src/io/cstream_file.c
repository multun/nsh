#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>

static int file_io_reader(struct cstream *cs)
{
    return getc(((struct cstream_file *)cs)->file);
}

static void file_io_dest(struct cstream *base_cs)
{
    struct cstream_file *cs = (struct cstream_file *)base_cs;
    if (cs->close_on_exit)
        fclose(cs->file);
}

struct io_backend io_file_backend = {
    .reader = file_io_reader,
    .dest = file_io_dest,
};

void cstream_file_init(struct cstream_file *cs, FILE *stream, bool close_on_exit)
{
    cstream_init(&cs->base, &io_file_backend, false);
    cs->close_on_exit = close_on_exit;
    cs->file = stream;
}
