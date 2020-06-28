#include "shexec/value.h"
#include "utils/alloc.h"

#include <assert.h>
#include <stdlib.h>


static void sh_string_free(struct sh_string *string)
{
    if (string->base.type != SH_VALUE_CONST_STRING)
        free(string->__str);
}


static void sh_value_free(struct sh_value *shval)
{
    switch (shval->type) {
    case SH_VALUE_STRING:
    case SH_VALUE_CONST_STRING:
        sh_string_free((struct sh_string*)shval);
        break;
    default:
        abort();
    }

    free(shval);
}


STATIC_REF_DEFINE(struct sh_value, sh_value, refcnt, sh_value_free)


struct sh_string *sh_string_create(char *str)
{
    struct sh_string *res = zalloc(sizeof(*res));
    sh_value_init(&res->base, SH_VALUE_STRING);
    res->__str = str;
    return res;
}


struct sh_string *sh_const_string_create(char *str)
{
    struct sh_string *res = zalloc(sizeof(*res));
    sh_value_init(&res->base, SH_VALUE_CONST_STRING);
    res->__str = str;
    return res;
}
