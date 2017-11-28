#pragma once

#include <stddef.h>

typedef struct evect
{
  size_t size;
  size_t capacity;
  char *data;
} s_evect;


void evect_init(s_evect *vect, size_t capacity);
void evect_destroy(s_evect *vect);

void evect_push(s_evect *vect, char c);
