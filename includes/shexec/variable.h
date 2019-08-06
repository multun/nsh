#pragma once

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
