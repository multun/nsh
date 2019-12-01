#pragma once

#include "utils/evect.h"
#include "shexp/expansion.h"

#include <stdbool.h>

enum glob_flags
{
    GLOB_FLAGS_PERIOD = 1,
};

enum glob_status
{
    GLOB_MATCH = 0,
    GLOB_NOMATCH = 1,
    GLOB_INVALID = 2,
};

enum glob_type
{
    GLOB_TYPE_TRIVIAL = 0,
    GLOB_TYPE_COMPLEX = 1,
    GLOB_TYPE_INVALID = 2,
};

struct glob_pattern
{
    const char *data;
    const char *meta;
};

enum glob_status glob_match(struct glob_pattern *pattern, size_t i, const char *string, int flags);

struct glob_path_element
{
    size_t offset;
    size_t length;
    size_t sep_count;

    char *name;
    /* a path element is trivial if it contains no special globbing char */
    bool trivial;
};

#include "utils/pvect.h"

#define GVECT_NAME gpath_vect
#define GVECT_TYPE struct glob_path_element *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE


struct glob_state
{
    struct gpath_vect path_elements;
    struct evect path_buffer;
};

#define DEFAULT_PATH_ELEMENTS 10

static inline void glob_state_init(struct glob_state *state)
{
    gpath_vect_init(&state->path_elements, DEFAULT_PATH_ELEMENTS);
    evect_init(&state->path_buffer, 42);
}

static inline void glob_state_reset(struct glob_state *state)
{
    gpath_vect_reset(&state->path_elements);
    evect_reset(&state->path_buffer);
}

static inline void glob_state_destroy(struct glob_state *state)
{
    gpath_vect_destroy(&state->path_elements);
    evect_destroy(&state->path_buffer);
}

void glob_expand(struct glob_state *glob_state, struct expansion_result *result,
                 struct expansion_callback_ctx *callback);
