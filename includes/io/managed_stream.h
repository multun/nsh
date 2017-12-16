#pragma once

#include "io/cstream.h"

#include <stdio.h>


struct arg_context;


int cstream_dispatch_init(struct context *context, s_cstream **cs,
                          struct arg_context *arg_cont);
