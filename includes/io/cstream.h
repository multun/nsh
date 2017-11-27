#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef enum cstream_backend
{
  CSTREAM_BACKEND_FILE,
  CSTREAM_BACKEND_STRING,
} e_cstream_backend;


typedef struct lineinfo
{
  size_t line;
  size_t column;
  char *source;
} s_lineinfo;


#define LINEINFO(Source) \
  (s_lineinfo)           \
  {                      \
    .line = 1,           \
    .column = 1,         \
    .source = (Source),  \
  }


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
} s_cstream;


void cstream_init_file(s_cstream *cs, FILE *stream, char *source);
void cstream_init_string(s_cstream *cs, char *string, char *source);
int cstream_peek(s_cstream *cs);
int cstream_pop(s_cstream *cs);
