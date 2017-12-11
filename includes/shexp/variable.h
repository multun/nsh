#pragma once

#include <stdbool.h>

typedef struct variable
{
  bool to_export;
  bool touched;
  char *value;
} s_var;

#define VARIABLE(Export, Touched, Value)                      \
((s_var)                                                      \
{                                                             \
  .to_export = Export,                                        \
  .touched = Touched,                                         \
  .value = Value,                                             \
}
