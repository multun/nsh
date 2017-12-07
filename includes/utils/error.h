#pragma once

#include "utils/lineinfo.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>


typedef struct ex_class
{
  // this structure isn't useful for anything but to use the
  // linker as a way to differentiate exceptions
  char reserved[1];
} s_ex_class;


typedef struct keeper
{
  struct keeper *father;
  jmp_buf env;
} s_keeper;


typedef struct errman
{
  const s_ex_class *class;
} s_errman;


typedef struct errcont
{
  struct errman *errman;
  struct keeper *keeper;
} s_errcont;



#define ERRMAN                                  \
  (s_errman)                                    \
  {                                             \
    .class = NULL,                              \
  }


#define ERRCONT(Man, Keeper)                    \
  (s_errcont)                                   \
  {                                             \
    .errman = (Man), .keeper = (Keeper)         \
  }


#define KEEPER(Father)                          \
  (s_keeper)                                    \
  {                                             \
    .father = (Father)                          \
  }


void shraise(s_errcont *cont, const s_ex_class *class);
int sherror(const s_lineinfo *lineinfo, s_errcont *cont,
            const char *format, ...);
