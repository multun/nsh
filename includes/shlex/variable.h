#pragma once

#include <assert.h>
#include <ctype.h>

#include "utils/attr.h"
#include "utils/evect.h"
#include <stdbool.h>


typedef struct variable
{
    bool to_export;
    char *value;
} s_var;

#define VARIABLE(Value)                                                                  \
    ((s_var){                                                                            \
        .to_export = false,                                                              \
        .value = Value,                                                                  \
    })


struct variable_name {
    struct evect simple_var;
    bool is_special;
};

__unused static bool is_basic(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

__unused static bool is_exp_special(char c)
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

__unused static bool simple_variable_name_check(struct evect *var, char c) {
    if (var == NULL || var->size == 0)
        return is_basic(c);

    return is_basic(c) || isdigit(c);
}


__unused static bool variable_name_check(struct variable_name *var, char c) {
    if (var->is_special)
        // ${#foo} isn't valid
        return false;

    if (var->simple_var.size == 0 && (isdigit(c) || is_exp_special(c)))
        return true;

    return simple_variable_name_check(&var->simple_var, c);
}

__unused static void simple_variable_name_push(struct evect *var, char c)
{
    assert(simple_variable_name_check(var, c));
    evect_push(var, c);
}

__unused static void variable_name_push(struct variable_name *var, char c)
{
    assert(variable_name_check(var, c));
    if (var->simple_var.size == 0 && (isdigit(c) || is_exp_special(c)))
        var->is_special = true;

    simple_variable_name_push(&var->simple_var, c);
}

__unused static void simple_variable_name_destroy(struct evect *var)
{
    evect_destroy(var);
}

__unused static void variable_name_destroy(struct variable_name *var)
{
    simple_variable_name_destroy(&var->simple_var);
}

__unused static void variable_name_init(struct variable_name *var, size_t size)
{
    var->is_special = false;
    evect_init(&var->simple_var, size);
}

__unused static void simple_variable_name_finalize(struct evect *var)
{
    evect_push(var, '\0');
}

__unused static void variable_name_finalize(struct variable_name *var)
{
    simple_variable_name_finalize(&var->simple_var);
}
