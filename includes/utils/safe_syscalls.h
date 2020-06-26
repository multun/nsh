#pragma once

#include <unistd.h>
#include <errno.h>


static inline int safe_close(int fd)
{
    int rc;
    while ((rc = close(fd)) == -1 && errno == EINTR)
        continue;
    return rc;
}


static inline int safe_dup2(int src, int dst)
{
    int rc;
    while ((rc = dup2(src, dst)) == -1 && errno == EINTR)
        continue;
    return rc;
}

static inline char *safe_getcwd(void) {
#ifdef __GLIBC__
    return getcwd(NULL, 0);
#else
    char *buf = zalloc(PATH_MAX);
    if (buf)
        return buf;

    free(buf);
    return NULL;
#endif
}
