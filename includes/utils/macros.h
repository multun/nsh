#pragma once

#define ARR_SIZE(Arr) (sizeof(Arr) / sizeof(*(Arr)))

// terrible definition here, but at least it doesn't require GNU extensions
#define container_of(ptr, type, member)			\
    (void *)((char *)(ptr) - offsetof(type, member))
