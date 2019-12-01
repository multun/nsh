#include "io/cstream.h"
#include "utils/alloc.h"

#include <stdlib.h>

void cstream_init(struct cstream *cs, struct io_backend *backend, bool interactive)
{
    *cs = (struct cstream){
        .backend = backend,
        .interactive = interactive,
        .has_buf = false,
        .offset = 0,
    };
}

bool cstream_eof(struct cstream *cs)
{
    return cstream_peek(cs) == EOF;
}

static inline int cstream_get(struct cstream *cs)
{
    return cs->backend->reader(cs);
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

    cs->offset++;
    return res;
}

void cstream_destroy(struct cstream *cs)
{
    if (cs->backend->dest)
        cs->backend->dest(cs);
}
