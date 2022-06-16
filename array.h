#pragma once

#include <stddef.h>

// Init with = {0} and cleanup with free(ptr).
typedef struct {
    char  *ptr;
    size_t len;
} array;

// Return a pointer to more uninitialized bytes appended to the end of the array.
char* array_grow(array*, size_t more);
