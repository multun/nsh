#include <nsh_io/cstream.h>
#include <nsh_utils/alloc.h>

#include <stdio.h>

static int string_io_reader(struct cstream *cs)
{
    struct cstream_string *css = (struct cstream_string *)cs;

    const char *str = css->string;
    if (!*str)
        return EOF;

    /* using an unsigned char is required to avoid sign extension */
    unsigned char res = *(str++);
    css->string = str;
    return res;
}

static struct io_backend io_string_backend = {
    .reader = string_io_reader,
    .dest = NULL,
};

void cstream_string_init(struct cstream_string *cs, const char *string) {
    cstream_init(&cs->base, &io_string_backend, false);
    cs->string = string;
}
