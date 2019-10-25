#pragma once

struct refcnt;

typedef void (*refcnt_free_f)(struct refcnt *refcnt);

struct refcnt
{
    int count;
    refcnt_free_f free;
};

void ref_init(struct refcnt *refcnt, refcnt_free_f free);
void ref_get(struct refcnt *refcnt);
void ref_put(struct refcnt *refcnt);
