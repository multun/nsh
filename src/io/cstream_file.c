#include <nsh_io/cstream.h>
#include <nsh_utils/alloc.h>

#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>

int cstream_file_setup(FILE **file, const char *path, bool missing_ok)
{
    if (!(*file = fopen(path, "r"))) {
        int res = errno;
        if (missing_ok && res == ENOENT)
            return res;
        // TODO: check the return code is right
        warn("cannot open input script");
        return res;
    }

    if (fcntl(fileno(*file), F_SETFD, FD_CLOEXEC) < 0) {
        int res = errno;
        warn("couldn't set CLOEXEC on input file %d", fileno(*file));
        return res;
    }

    return 0;
}

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
