#include <nsh_utils/refcnt.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>


void ref_init(struct refcnt *refcnt, refcnt_free_f free)
{
    assert(refcnt->count == 0);
    refcnt->free = free;
}

void ref_get(struct refcnt *refcnt)
{
    assert(refcnt->count >= 0);
    refcnt->count++;
}

void ref_put(struct refcnt *refcnt)
{
    assert(refcnt->count >= 0);

    if (--refcnt->count >= 0)
        return;
    if (refcnt->free)
        refcnt->free(refcnt);
    else
        free(refcnt);
}
