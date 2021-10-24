#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <nsh_utils/evect.h>


char *home_suffix(const char *suffix);

struct path_component
{
    size_t comp_i;
    const char *data;
    size_t length;
    bool root;
};

static inline bool path_component_trailing_slash(struct path_component *cmp)
{
    return cmp->data[cmp->length] == '/';
}

void path_component_init(struct path_component *cmp, const char *data);
void path_component_next(struct path_component *cmp);
bool path_component_valid(struct path_component *cmp);
void path_component_repr(struct path_component *cmp, struct evect *dst);
size_t path_component_repr_size(struct path_component *cmp);

char *path_canonicalize(const char *complex_path);

char *path_join(const char *base, const char *relpath);
size_t path_count_components(const char *path);
const char *path_skip_components(const char *path, size_t count);
const char *path_remove_prefix(const char *path, const char *prefix_path);

struct pathlist_iter
{
    const char *data;
    size_t length;
};

static inline void pathlist_iter_scan(struct pathlist_iter *it)
{
    for (it->length = 0; it->data[it->length]; it->length++)
        if (it->data[it->length] == ':')
            break;
}

static inline void pathlist_iter_init(struct pathlist_iter *it, const char *str)
{
    it->data = str;
    pathlist_iter_scan(it);
}

static inline bool pathlist_iter_valid(struct pathlist_iter *it)
{
    return *it->data != '\0';
}

static inline void pathlist_iter_next(struct pathlist_iter *it)
{
    it->data += it->length;
    while (*it->data == ':')
        it->data++;
    pathlist_iter_scan(it);
}
