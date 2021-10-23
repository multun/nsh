#pragma once

#include "io/cstream.h"


struct cstream_string {
    struct cstream base;
    const char *string;
};

void cstream_string_init(struct cstream_string *cstream, const char *string);
