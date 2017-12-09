#pragma once

#include "io/cstream.h"

#include <stdio.h>


typedef struct managed_stream
{
  FILE *in_file;
  s_cstream *cs;
} s_managed_stream;


void managed_stream_init(struct context *context, struct managed_stream *ms,
                         int argc, char *argv[]);
void managed_stream_destroy(struct managed_stream *ms);
