#pragma once

#include <nsh_utils/hashmap.h>


struct hashmap_it
{
    size_t i;
    struct hashmap_item *cur;
};

#define HASH_TABLE_IT_INIT                                                               \
    (struct hashmap_it)                                                                  \
    {                                                                                    \
        .i = 0                                                                           \
    }

#define for_each_hash(It, Hash)                                                          \
    for ((It) = HASH_TABLE_IT_INIT; (It).i < (Hash)->capacity; (It).i++)                 \
        for ((It).cur = (Hash)->tab[(It).i]; (It).cur; (It).cur = (It).cur->next)

struct hashmap_safe_it
{
    size_t i;
    struct hashmap_item *cur;
    struct hashmap_item *tmp;
};

#define HASH_TABLE_SAFE_IT_INIT                                                          \
    (struct hashmap_safe_it)                                                             \
    {                                                                                    \
        .i = 0                                                                           \
    }

#define for_each_hash_safe(It, Hash)                                                     \
    for ((It) = HASH_TABLE_SAFE_IT_INIT; (It).i < (Hash)->capacity; (It).i++)            \
        for ((It).cur = (Hash)->tab[(It).i]; (It).cur; (It).cur = (It).tmp)              \
            if (((It).tmp = (It).cur->next) || 1)
