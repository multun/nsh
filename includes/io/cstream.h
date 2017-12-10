#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "utils/evect.h"
#include "utils/lineinfo.h"


struct cstream;
struct context;

typedef int (*f_io_reader)(struct cstream *cs);
typedef void (*f_io_destructor)(struct cstream *cs);


/**
** \brief io backends are generic to avoid code duplication
*/
typedef struct io_backend
{
  f_io_reader reader;
  f_io_destructor dest;
} s_io_backend;


/**
** \brief a cstream is a character stream, similar to FILE*
** \desc it can also read from a readline or string input
*/
typedef struct cstream
{
  void *data;
  bool interactive;
  struct context *context;
  s_io_backend *backend;
  // a backend-dependant cursor
  size_t back_pos;
  // the line-related informations
  s_lineinfo line_info;
  // whether a read character is waiting to be poped
  bool has_buf;
  int buf;
  bool eof;
  // contains each poped character
  s_evect linebuf;
} s_cstream;


s_cstream *cstream_create_base(void);
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
