#pragma once

#include <stdbool.h>


struct static_refcnt
{
    int count;
};

static inline void static_ref_init(struct static_refcnt *refcnt)
{
    refcnt->count = 0;
}

static inline void static_ref_get(struct static_refcnt *refcnt)
{
    refcnt->count++;
}

static inline bool static_ref_put(struct static_refcnt *refcnt)
{
    refcnt->count--;
    return refcnt->count == -1;
}


#define STATIC_REFCNT_DEFINE(Struct, Name, Field, Free)    \
    static inline void Name ## _get(Struct *item)       \
    {                                                   \
        static_ref_get(&item->Field);                   \
    }                                                   \
                                                        \
    static inline void Name ## _put(Struct *item)       \
    {                                                   \
        if (!item)                                      \
            return;                                     \
                                                        \
        if (static_ref_put(&item->Field))               \
            Free(item);                                 \
    }
