#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>

void cstream_init(struct cstream *cs, struct io_backend *backend, bool interactive)
{
    cs->interactive = interactive;
    cs->backend = backend;
    cs->has_buf = false;
    cs->eof = false;
}

bool cstream_eof(struct cstream *cs)
{
    return !(cs->has_buf && cs->buf != EOF) && cs->eof;
}

static inline int cstream_get(struct cstream *cs)
{
    int res = cs->backend->reader(cs);
    if (res == EOF)
        cs->eof = true;

    return res;
}

int cstream_peek(struct cstream *cs)
{
    if (!cs->has_buf) {
        cs->has_buf = true;
        cs->buf = cstream_get(cs);
    }

    return cs->buf;
}

int cstream_pop(struct cstream *cs)
{
    int res;

    if (cs->has_buf) {
        cs->has_buf = false;
        res = cs->buf;
    } else
        res = cstream_get(cs);

    if (res == '\n') {
        cs->line_info.line++;
        cs->line_info.column = 1;
    } else
        cs->line_info.column++;

    return res;
}

void cstream_destroy(struct cstream *cs)
{
    if (cs->backend->dest)
        cs->backend->dest(cs);
}
