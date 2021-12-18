#include <nsh_io/cstream.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/logging.h>
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

nsh_err_t cstream_free(struct cstream *cs)
{
    nsh_err_t err = cstream_destroy(cs);
    free(cs);
    return err;
}

int cstream_eof(struct cstream *cs)
{
    int rc;
    if ((rc = cstream_peek(cs)) < 0)
        return rc;
    return rc == CSTREAM_EOF;
}

static inline int cstream_get(struct cstream *cs)
{
    int rc;
    nsh_trace("started reading...");
    if ((rc = cs->backend->reader(cs)) < 0) {
        nsh_debug("read failed");
        return rc;
    }
    nsh_debug("read `%c' (%d)", rc, rc);
    return rc;
}

int cstream_peek(struct cstream *cs)
{
    if (cs->has_buf)
        return cs->buf;

    int res;
    if ((res = cstream_get(cs)) < 0)
        return res;
    cs->buf = res;
    cs->has_buf = true;
    return res;
}

int cstream_pop(struct cstream *cs)
{
    int res;

    if (cs->has_buf) {
        cs->has_buf = false;
        res = cs->buf;
    } else {
        if ((res = cstream_get(cs)) < 0)
            return res;
    }

    if (res == '\n') {
        cs->line_info.line++;
        cs->line_info.column = 1;
    } else
        cs->line_info.column++;

    cs->offset++;
    return res;
}

void cstream_discard(struct cstream *cs)
{
    assert(cs->has_buf);
    cs->has_buf = false;
}


nsh_err_t cstream_destroy(struct cstream *cs)
{
    if (cs->backend->dest)
        return cs->backend->dest(cs);
    return NSH_OK;
}

nsh_err_t cstream_reset(struct cstream *cs)
{
    cs->has_buf = false;
    if (cs->backend->reset)
        return cs->backend->reset(cs);
    return NSH_OK;
}
