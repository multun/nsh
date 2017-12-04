#pragma once

#include "io/cstream.h"

typedef int (*f_stream_consumer)(s_cstream *cs);

int repl(f_stream_consumer consumer);
