#pragma once

#include "io/cstream.h"

#include <stdio.h>


/**
** \brief setups up and destroys an underlying typeful stream.
*/
typedef struct managed_stream
{
  FILE *in_file;
  s_cstream *cs;
} s_managed_stream;


int managed_stream_init(struct context *context, struct managed_stream *ms,
                        int argc, char *argv[]);
void managed_stream_destroy(struct managed_stream *ms);
