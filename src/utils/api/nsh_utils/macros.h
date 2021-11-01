#pragma once

#include <stddef.h>

#define NEEDED_STORAGE(I, S) (((I) + (S)-1) / (S))

#define ARR_SIZE(Arr) (sizeof(Arr) / sizeof(*(Arr)))

#define container_of(ptr, type, member) (void *)((char *)(ptr)-offsetof(type, member))


#define XSTRINGIFY(s) STRINGIFY(s)
#define STRINGIFY(s) #s
