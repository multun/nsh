#pragma once

#include <assert.h>
#include <ctype.h>

#include "utils/attr.h"
#include "utils/evect.h"
#include <stdbool.h>



struct variable_name {
    struct evect simple_var;
    bool is_special;
};

static inline bool is_basic(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline bool is_exp_special(char c)
{
    switch (c) {
    case '?':
    case '@':
    case '*':
    case '$':
    case '#':
        return true;
    default:
        return false;
    }
}

static inline bool simple_variable_name_check_first(char c)
{
    return is_basic(c);
}

static inline bool simple_variable_name_check_body(char c)
{
    return is_basic(c) || isdigit(c);
}

static inline bool simple_variable_name_check_at(size_t i, char c)
{
    return (i == 0
            ? simple_variable_name_check_first
            : simple_variable_name_check_body)(c);
}

static inline bool simple_variable_name_check(struct evect *var, char c) {
    if (var == NULL || var->size == 0)
        return simple_variable_name_check_first(c);

    return simple_variable_name_check_body(c);
}


static inline bool variable_name_check(struct variable_name *var, char c) {
    if (var->is_special)
        // ${#foo} isn't valid
        return false;

    if (var->simple_var.size == 0 && (isdigit(c) || is_exp_special(c)))
        return true;

    return simple_variable_name_check(&var->simple_var, c);
}

static inline int variable_name_check_string(const char *str, size_t size)
{
    if (size == 0)
        return 1;

    for (size_t i = 0; i < size; i++)
    {
        if (!simple_variable_name_check_at(i, str[i]))
            return 1;
    }
    return 0;
}


static inline void simple_variable_name_push(struct evect *var, char c)
{
    evect_push(var, c);
}

static inline void variable_name_push(struct variable_name *var, char c)
{
    assert(variable_name_check(var, c));
    if (var->simple_var.size == 0 && (isdigit(c) || is_exp_special(c)))
        var->is_special = true;

    simple_variable_name_push(&var->simple_var, c);
}

static inline void simple_variable_name_destroy(struct evect *var)
{
    evect_destroy(var);
}

static inline void variable_name_destroy(struct variable_name *var)
{
    simple_variable_name_destroy(&var->simple_var);
}

static inline void variable_name_init(struct variable_name *var, size_t size)
{
    var->is_special = false;
    evect_init(&var->simple_var, size);
}

static inline void simple_variable_name_finalize(struct evect *var)
{
    evect_push(var, '\0');
}

static inline void variable_name_finalize(struct variable_name *var)
{
    simple_variable_name_finalize(&var->simple_var);
}
