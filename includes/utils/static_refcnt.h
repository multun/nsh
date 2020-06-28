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


#define STATIC_REF_DECLARE(Struct, Name)        \
    void Name ## _get(Struct*);                 \
    void Name ## _put(Struct*);


#define STATIC_REF_DEFINE(Struct, Name, Field, Free)    \
    void Name ## _get(Struct *item)                     \
    {                                                   \
        static_ref_get(&item->Field);                   \
    }                                                   \
                                                        \
    void Name ## _put(Struct *item)                     \
    {                                                   \
        if (static_ref_put(&item->Field))               \
            Free(item);                                 \
    }
