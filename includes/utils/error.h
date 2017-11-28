#pragma once

#include "utils/lineinfo.h"


typedef struct sherror
{
  s_lineinfo lineinfo;
  char *message;
} s_sherror;


#define SHERROR(Lineinfo, Message)              \
  (s_sherror)                                   \
  {                                             \
    .lineinfo = (Lineinfo),                     \
    .message = (Message),                       \
  }


s_sherror *sherror_alloc(s_lineinfo *lineinfo, char *message);
s_sherror *sherror_free(s_sherror *error);
