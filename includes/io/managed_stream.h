#pragma once

#include "io/cstream.h"

#include <stdio.h>


int cstream_dispatch_init(struct context *context, s_cstream **cs,
                          int argc, char *argv[]);
