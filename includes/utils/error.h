#pragma once

#include "utils/lineinfo.h"

#include <stdbool.h>
#include <stdio.h>


typedef struct errman
{
  bool panic;
} s_errman;


#define ERRMAN_FAILING(Errman) ((Errman)->panic)


#define ERRMAN      \
  (s_errman)        \
  {                 \
    .panic = false, \
  }


int sherror(s_lineinfo *lineinfo, s_errman *errman, const char *format, ...);
