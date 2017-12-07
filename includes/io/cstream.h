#pragma once

#include <stdio.h>
#include <stdbool.h>

#include "utils/lineinfo.h"


struct cstream;

typedef int (*f_io_reader)(struct cstream *cs);
typedef void (*f_io_destructor)(struct cstream *cs);

typedef struct io_backend
{
  f_io_reader reader;
  f_io_destructor dest;
} s_io_backend;


typedef struct cstream
{
  void *data;
  s_io_backend *backend;
  size_t back_pos;
  s_lineinfo line_info;
  bool has_buf;
  int buf;
  bool eof;
} s_cstream;


s_cstream *cstream_from_file(FILE *stream, char *source);
s_cstream *cstream_readline(void);
s_cstream *cstream_from_string(char *string, char *source);

void cstream_free(s_cstream *cs);

int cstream_peek(s_cstream *cs);
int cstream_pop(s_cstream *cs);
bool cstream_eof(s_cstream *cs);


extern s_io_backend g_io_file_backend;
extern s_io_backend g_io_readline_backend;
extern s_io_backend g_io_string_backend;
