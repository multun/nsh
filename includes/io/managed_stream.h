#pragma once

#include "io/cstream.h"

#include <stdio.h>


struct managed_stream
{
  FILE *in_file;
  s_cstream *cs;
};


void managed_stream_init(struct managed_stream *ms, int argc, char *argv[]);
void managed_stream_destroy(struct managed_stream *ms);
