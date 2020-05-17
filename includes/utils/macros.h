#pragma once

#include <stddef.h>

#define ARR_SIZE(Arr) (sizeof(Arr) / sizeof(*(Arr)))

#define container_of(ptr, type, member)			\
    (void *)((char *)(ptr) - offsetof(type, member))
