#pragma once

#include <stddef.h>

extern int parse_digit(int c);
extern int parse_integer(size_t *res, size_t basis, const char *str, size_t max_length);

/* parse a nul-byte terminated integer */
extern int parse_pure_integer(size_t *res, size_t basis, const char *str);
