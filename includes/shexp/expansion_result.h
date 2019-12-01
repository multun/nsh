#pragma once

#include "utils/evect.h"

#include <string.h>
#include <assert.h>
#include <stdbool.h>

// each output byte has a combination of these two flags:
// it can start an unquoted section, and / or stop one.
// beware, a char can stop a section even though no section
// was opened.
enum expansion_meta {
    EXPANSION_UNQUOTED = 1,
    EXPANSION_END_UNQUOTED = 2,
};

struct expansion_result {
    /* the expanded bytes */
    struct evect string;

    /* each output byte has a corresponding metadata byte */
    struct evect metadata;
};


static inline size_t expansion_result_size(struct expansion_result *result)
{
    return evect_size(&result->string);
}

static inline void expansion_result_cut(struct expansion_result *result, size_t i)
{
    assert(i <= result->string.size);
    assert(i <= result->metadata.size);
    result->string.size = i;
    result->metadata.size = i;
}

static inline char expansion_result_get(struct expansion_result *result, size_t i)
{
    return evect_get(&result->string, i);
}

static inline char *expansion_result_data(struct expansion_result *result)
{
    return evect_data(&result->string);
}

static inline char *expansion_result_meta(struct expansion_result *result)
{
    return evect_data(&result->metadata);
}

static inline bool expansion_result_getflag(struct expansion_result *result, size_t i, int flag)
{
    return evect_get(&result->metadata, i) & flag;
}

static inline void expansion_result_setflag(struct expansion_result *result, size_t i, int flag)
{
    evect_data(&result->metadata)[i] |= flag;
}

static inline void expansion_result_reset(struct expansion_result *result)
{
    evect_reset(&result->string);
    evect_reset(&result->metadata);
}

static inline void expansion_result_init(struct expansion_result *result, size_t size)
{
    evect_init(&result->string, size);
    evect_init(&result->metadata, size);
}

static inline void expansion_result_destroy(struct expansion_result *result)
{
    evect_destroy(&result->string);
    evect_destroy(&result->metadata);
}

static inline void expansion_result_push(struct expansion_result *result, char c, int flags)
{
    evect_push(&result->string, c);
    evect_push(&result->metadata, flags);
}

static inline char *expansion_result_dup(struct expansion_result *result)
{
    return strndup(expansion_result_data(result), expansion_result_size(result));
}
