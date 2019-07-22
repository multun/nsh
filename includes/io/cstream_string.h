#pragma once

#include "io/cstream.h"


struct cstream_string {
    struct cstream base;
    char *string;
};

void cstream_string_init(struct cstream_string *cstream, char *string);
