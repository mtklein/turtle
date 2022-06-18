#pragma once

#include <stddef.h>

// Init with = {0} and cleanup with free(ptr).
struct array {
    char  *ptr;
    size_t len;
};

// Return a pointer to more uninitialized bytes appended to the end of the array.
void* array_grow(struct array*, size_t more);
