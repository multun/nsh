#pragma once

#include <stddef.h>
#include <assert.h>


struct ring_buffer_meta
{
    size_t start_index;
    size_t size;
};

static inline ring_buffer_init(struct ring_buffer_meta *meta)
{
    meta->start_index = 0;
    meta->size = 0;
}

static inline size_t ring_buffer_getindex(const struct ring_buffer_meta *meta,
                                          size_t capacity, size_t i)
{
    assert(i < meta->size);
    return (meta->start_index + i) % capacity;
}

static inline size_t ring_buffer_push(struct ring_buffer_meta *meta, size_t capacity)
{
    assert(meta->size < capacity);
    return ring_buffer_getindex(meta, capacity, meta->size++);
}

static inline size_t ring_buffer_pop(struct ring_buffer_meta *meta, size_t capacity)
{
    assert(meta->size > 0);
    size_t poped_index = ring_buffer_getindex(meta, capacity, 0);
    meta->size--;
    meta->start_index = (meta->start_index + 1) % capacity;
    return poped_index;
}
