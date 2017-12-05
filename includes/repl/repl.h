#pragma once

#include "io/cstream.h"
#include "utils/error.h"

typedef int (*f_stream_consumer)(s_cstream *cs, s_errman *errman);

int repl(f_stream_consumer consumer);
