#pragma once

#include "utils/macros.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


#define SIZE_T_BITS (sizeof(size_t) * CHAR_BIT)


struct char_bitset
{
    size_t data[NEEDED_STORAGE(UCHAR_MAX, SIZE_T_BITS)];
};


static inline void char_bitset_init(struct char_bitset *bitset)
{
    memset(&bitset->data, 0, sizeof(bitset->data));
}


static inline size_t char_bitset_slot(size_t i)
{
    return i / SIZE_T_BITS;
}

static inline size_t char_bitset_off(size_t i)
{
    return i % SIZE_T_BITS;
}

static inline bool char_bitset_get(struct char_bitset *bitset, unsigned char c)
{
    return !!(bitset->data[char_bitset_slot(c)] & (1uLL << char_bitset_off(c)));
}

static inline void char_bitset_set(struct char_bitset *bitset, unsigned char c, bool val)
{
    size_t *slot = &bitset->data[char_bitset_slot(c)];
    size_t offset = char_bitset_off(c);
    size_t mask = 1uLL << offset;
    size_t ival = val ? 1uLL : 0uLL;
    *slot = (*slot & ~mask) | ival << offset;
}
