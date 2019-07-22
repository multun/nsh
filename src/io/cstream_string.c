#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdio.h>

static int string_io_reader(struct cstream *cs)
{
    struct cstream_string *css = (struct cstream_string *)cs;

    char *str = css->string;
    if (!*str)
        return EOF;

    int res = *(str++);
    css->string = str;
    return res;
}

static struct io_backend io_string_backend = {
    .reader = string_io_reader,
    .dest = NULL,
};

void cstream_string_init(struct cstream_string *cs, char *string) {
    cs->base.backend = &io_string_backend;
    cs->base.interactive = false;
    cs->string = string;
}
