#pragma once

#include <stddef.h>


typedef struct buf
{
  size_t size;
  char data[];
} s_buf;


s_buf *buf_create(size_t size);
