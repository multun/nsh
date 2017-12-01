#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "utils/lineinfo.h"


typedef enum cstream_backend
{
  CSTREAM_BACKEND_FILE,
  CSTREAM_BACKEND_STRING,
} e_cstream_backend;


typedef struct cstream
{
  union
  {
    FILE *stream;
    char *string;
  } data;
  e_cstream_backend type;
  s_lineinfo line_info;
  bool has_buf;
  int buf;
  bool eof;
} s_cstream;


s_cstream *cstream_from_file(FILE *stream, char *source);
s_cstream *cstream_from_string(char *string, char *source);
void cstream_free(s_cstream *cs);

int cstream_peek(s_cstream *cs);
int cstream_pop(s_cstream *cs);
bool cstream_eof(s_cstream *cs);
