#pragma once

#include <stddef.h>

struct buffer {
    size_t len;
    char   bytes[];
};

// Create with buffer_grow(NULL, ...); clean up with free().
struct buffer* buffer_grow(struct buffer*, size_t more);

#if defined(UNITY_BUILD)
    #include "buffer.c"
    #define UNITY_BUILD 1
#endif
