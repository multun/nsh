#pragma once

#include "utils/static_refcnt.h"

struct sh_value
{
    struct static_refcnt refcnt;
    enum sh_value_type
    {
        SH_VALUE_STRING = 0,
        SH_VALUE_CONST_STRING = 1,
    } type;
};

static inline bool sh_value_is_string(struct sh_value *val)
{
    switch (val->type) {
    case SH_VALUE_STRING:
    case SH_VALUE_CONST_STRING:
        return true;
    default:
        return false;
    }
}

#define STATIC_SH_VALUE(Type)                   \
    { .type = (Type) }


static inline void sh_value_init(struct sh_value *shval, enum sh_value_type type)
{
    static_ref_init(&shval->refcnt);
    shval->type = type;
}


struct sh_string
{
    struct sh_value base;
    /* __str shouldn't be modified by the user */
    char *__str;
};

#define STATIC_SH_STRING(Type)                   \
    { .type = (Type) }


static inline const char *sh_string_data(struct sh_string *shstr)
{
    return shstr->__str;
}

struct sh_string *sh_const_string_create(char *str);

struct sh_string *sh_string_create(char *str);

void sh_value_free(struct sh_value *shval);

STATIC_REFCNT_DEFINE(struct sh_value, sh_value, refcnt, sh_value_free)


static inline void sh_string_get(struct sh_string *str)
{
    sh_value_get(&str->base);
}

static inline void sh_string_put(struct sh_string *str)
{
    sh_value_put(&str->base);
}
